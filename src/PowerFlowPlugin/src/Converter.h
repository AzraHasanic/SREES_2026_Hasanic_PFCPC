#pragma once

#include <cmath>

#include <sparse/ISolver.h>
#include <td/String.h>
#include <atomic>
#include <functional>
#include <complex>
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
//  Converter – MATPOWER .m  →  dTwin .dmodl  (polar coordinates)
class Converter
{
public:
    struct Options
    {
        bool flatStart = true;
        bool includeLimits = false;
        bool zeroLoads = false;
    };

    struct Results
    {
        int buses = 0;
        int gens = 0;
        int branches = 0;
        int pqBuses = 0;
        int pvBuses = 0;
        int slacks = 0;
        td::String outputFile;
    };

    using LogCb = std::function<void(const td::String&)>;
    using DoneCb = std::function<void(bool ok, const td::String& msg, const Results& res)>;

    std::atomic<int>  progressPercent{ 0 };
    std::atomic<bool> stopRequested{ false };

    void convert(const td::String& inputFile,
        const td::String& outputFile,
        const Options& opts,
        LogCb             logCb,
        DoneCb            doneCb);

    Results _lastResults;

private:
    using Cmplx = std::complex<double>;
    using BusRow = std::vector<double>;
    using Matrix = std::vector<BusRow>;
    using YijMap = std::unordered_map<int64_t, Cmplx>;

    static double  evalExpr(const std::string& raw);
    static int64_t yijKey(int i, int j) { return (int64_t)i * 1000000 + j; }
    static int64_t branchKey(int i, int j) { return (int64_t)100000000 * i + j; }

    static Matrix parseBlock(const std::vector<std::string>& lines,
        size_t startIdx, int elementType, int& outCount);

    static std::string dbl(double v);
};

//  Implementacija 

inline std::string Converter::dbl(double v)
{
    std::ostringstream ss;
    ss.precision(15);
    ss << v;
    return ss.str();
}

//vraca defaultVal ako indeks ne postoji
static inline double rowGet(const std::vector<double>& row, size_t idx, double defaultVal = 0.0)
{
    return (idx < row.size()) ? row[idx] : defaultVal;
}

inline double Converter::evalExpr(const std::string& raw)
{
    if (raw.empty()) return 0.0;
    std::string s = raw;

    for (size_t p = 0; (p = s.find("Inf", p)) != std::string::npos; p += 5)
        s.replace(p, 3, "1e100");

    char* end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    if (end && *end == '\0') return v;

    struct Eval {
        const char* p;
        double number() {
            while (*p == ' ') ++p;
            if (*p == '(') { ++p; double v = expr(); if (*p == ')') ++p; return v; }
            bool neg = false;
            if (*p == '-') { neg = true; ++p; }
            else if (*p == '+') ++p;
            struct Fn { const char* nm; double(*fn)(double); };
            Fn fns[] = { {"sqrt",std::sqrt},{"abs",std::abs},
                        {"sin",std::sin},{"cos",std::cos},
                        {"tan",std::tan},{"exp",std::exp},{"log",std::log} };
            for (auto& f : fns) {
                size_t len = strlen(f.nm);
                if (strncmp(p, f.nm, len) == 0 && p[len] == '(') {
                    p += len + 1; double a = expr(); if (*p == ')') ++p;
                    return neg ? -f.fn(a) : f.fn(a);
                }
            }
            if (strncmp(p, "pi", 2) == 0 && !std::isalpha((unsigned char)p[2])) {
                p += 2; return neg ? -M_PI : M_PI;
            }
            char* e = nullptr;
            double val = std::strtod(p, &e); p = e;
            return neg ? -val : val;
        }
        double power() {
            double b = number();
            while (*p == '^') { ++p; double e = number(); b = std::pow(b, e); }
            return b;
        }
        double term() {
            double v = power();
            while (*p == '*' || *p == '/') { char op = *p++; double r = power(); v = (op == '*') ? v * r : v / r; }
            return v;
        }
        double expr() {
            double v = term();
            while (*p == '+' || *p == '-') { char op = *p++; double r = term(); v = (op == '+') ? v + r : v - r; }
            return v;
        }
    };
    Eval e; e.p = s.c_str();
    return e.expr();
}

inline Converter::Matrix Converter::parseBlock(
    const std::vector<std::string>& lines,
    size_t startIdx, int elementType, int& outCount)
{
    Matrix data;
    data.reserve(512);
    size_t i = startIdx + 1;
    while (i < lines.size())
    {
        std::string line = lines[i++];
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
            line.pop_back();
        if (line.empty()) continue;
        size_t nw = line.find_first_not_of(" \t");
        if (nw == std::string::npos || line[nw] == '%') continue;
        if (line.find("];") != std::string::npos) break;
        auto cp = line.find('%');
        if (cp != std::string::npos) line = line.substr(0, cp);
        while (!line.empty() && (line.back() == ';' || line.back() == ' ')) line.pop_back();
        std::istringstream iss(line);
        std::string tok;
        BusRow row;
        while (iss >> tok) row.push_back(evalExpr(tok));
        if (row.empty()) continue;
        // Provjeri minimalni broj kolona
        if (elementType == 0 && row.size() < 9)  continue;  // bus: treba >= 9 kolona
        if (elementType == 1 && row.size() < 8)  continue;  // gen: treba >= 8 kolona
        if (elementType == 2 && row.size() < 11) continue;  // branch: treba >= 11 kolona
        if (elementType == 0) {
            if ((int)row[1] == 2) row[1] = 99.0;
        }
        else if (elementType == 1) {
            if (row[7] < 0.1) continue;
        }
        else {
            if (row[10] < 0.1) continue;
        }
        data.push_back(std::move(row));
    }
    outCount = (int)data.size();
    return data;
}

inline void Converter::convert(
    const td::String& inputFile,
    const td::String& outputFile,
    const Options& opts,
    LogCb             logCb,
    DoneCb            doneCb)
{
    auto log = [&](const char* msg) { if (logCb) logCb(td::String(msg)); };
    Results results;
    auto fail = [&](const char* msg) { if (doneCb) doneCb(false, td::String(msg), results); };

    progressPercent = 0;

    // Citanje .m file-a
    std::vector<std::string> lines;
    {
        std::ifstream f(inputFile.c_str());
        if (!f.is_open()) { fail("GRESKA: Ne mogu otvoriti ulazni fajl."); return; }
        std::string l;
        while (std::getline(f, l)) lines.push_back(l);
    }
    log("Fajl ucitan.");
    progressPercent = 5;
    if (stopRequested) { fail("Zaustavljeno."); return; }

    // Parsiranje zaglavlja
    double baseMVA = 0.0;
    Matrix busMat, genMat, branchMat;
    int nBuses = 0, nGens = 0, nBranches = 0;

    for (size_t i = 0; i < lines.size(); ++i)
    {
        const std::string& ln = lines[i];
        if (ln.find("mpc.baseMVA") == 0) {
            auto eq = ln.find('=');
            if (eq != std::string::npos) {
                std::string expr = ln.substr(eq + 1);
                auto sc = expr.find(';'); if (sc != std::string::npos) expr = expr.substr(0, sc);
                auto cm = expr.find('%'); if (cm != std::string::npos) expr = expr.substr(0, cm);
                baseMVA = evalExpr(expr);
            }
        }
        else if (ln.find("mpc.bus =") == 0 || ln.find("mpc.bus=") == 0)
            busMat = parseBlock(lines, i, 0, nBuses);
        else if (ln.find("mpc.gen =") == 0)
            genMat = parseBlock(lines, i, 1, nGens);
        else if (ln.find("mpc.branch =") == 0 || ln.find("mpc.branch=") == 0)
            branchMat = parseBlock(lines, i, 2, nBranches);
    }

    if (baseMVA == 0.0) { fail("GRESKA: baseMVA nije pronadjen ili je nula."); return; }
    if (nBuses == 0) { fail("GRESKA: Nema podataka o cvorovima."); return; }

    {
        td::String msg;
        msg.format("Parsirano: %d cvorova, %d generatora, %d grana.", nBuses, nGens, nBranches);
        log(msg.c_str());
    }
    progressPercent = 15;
    if (stopRequested) { fail("Zaustavljeno."); return; }

    // index map
    int n = nBuses;
    std::unordered_map<int, int> busIdToIdx;
    std::vector<int>            idxToId(n);
    for (int i = 0; i < n; ++i) {
        int bid = (int)busMat[i][0];
        busIdToIdx[bid] = i;
        idxToId[i] = bid;
    }

    // cvorovi
    const int PQ = 1, PV = 2, SLACK = 3;
    std::unordered_map<int, std::vector<BusRow>> genByBus;
    for (auto& g : genMat) {
        int bid = (int)g[0];
        genByBus[bid].push_back(g);
        auto itG = busIdToIdx.find(bid);
        if (itG == busIdToIdx.end()) continue;
        int idx = itG->second;
        if ((int)busMat[idx][1] == 99) busMat[idx][1] = PV;
    }

    std::vector<int> pqNodes, pvNodes, slackNodes;
    for (int i = 0; i < n; ++i) {
        int bt = (int)busMat[i][1];
        int bid = idxToId[i];
        if (bt == 99) { busMat[i][1] = PQ; bt = PQ; }
        if (bt == PQ)    pqNodes.push_back(bid);
        else if (bt == PV)    pvNodes.push_back(bid);
        else if (bt == SLACK) slackNodes.push_back(bid);
    }
    if (slackNodes.empty()) { fail("GRESKA: Nema slack cvora!"); return; }
    progressPercent = 20;

    // Y-matrica 
    log("Gradim Y-matricu (natID sparse)...");
    int estNZ = n + 2 * nBranches + 16;
    sparse::CmplxSolverReleaser pYSolver(
        sparse::createCmplxSolver(n, estNZ,
            sparse::Symmetry::NonSymmetric,
            sparse::SolverType::LU));
    Cmplx czero(0.0, 0.0);
    pYSolver->populateDiagonals(czero);

    std::vector<Cmplx> Y_ii(n, czero);
    YijMap             Y_ij;
    std::vector<std::set<int>> nodeBranches(n);
    std::set<int64_t> tapChangers, phaseShifters;

    for (auto& row : branchMat)
    {
        int fBus = (int)row[0], tBus = (int)row[1];
        double r = rowGet(row, 2), x = rowGet(row, 3), b = rowGet(row, 4);
        double tap = rowGet(row, 8, 1.0); if (tap == 0.0) tap = 1.0;
        double angDeg = rowGet(row, 9);

        if (tap != 1.0)   tapChangers.insert(branchKey(fBus, tBus));
        if (angDeg != 0.0) phaseShifters.insert(branchKey(fBus, tBus));

        auto itF = busIdToIdx.find(fBus);
        auto itT = busIdToIdx.find(tBus);
        if (itF == busIdToIdx.end() || itT == busIdToIdx.end()) continue;
        int fi = itF->second, ti = itT->second;
        nodeBranches[fi].insert(ti);
        nodeBranches[ti].insert(fi);

        Cmplx y = (r == 0 && x == 0) ? czero : Cmplx(1.0) / Cmplx(r, x);
        Cmplx bsh(0.0, b / 2.0);
        double angRad = angDeg * M_PI / 180.0;
        Cmplx  a(tap * std::cos(angRad), tap * std::sin(angRad));

        Cmplx dii_f = (y + bsh) / (a * std::conj(a));
        Cmplx dii_t = y + bsh;
        Cmplx dij = -y / std::conj(a);
        Cmplx dji = -y / a;

        Y_ii[fi] += dii_f;  Y_ii[ti] += dii_t;
        Y_ij[yijKey(fi, ti)] += dij;
        Y_ij[yijKey(ti, fi)] += dji;

        pYSolver->addTriple(fi, fi, dii_f);
        pYSolver->addTriple(ti, ti, dii_t);
        pYSolver->addTriple(fi, ti, dij);
        pYSolver->addTriple(ti, fi, dji);
    }


    // Shunt admitanse
    for (int i = 0; i < n; ++i) {
        double gs = rowGet(busMat[i], 4) / baseMVA, bs = rowGet(busMat[i], 5) / baseMVA;
        Cmplx sh(gs, bs);
        Y_ii[i] += sh;
        pYSolver->addTriple(i, i, sh);
    }

    progressPercent = 40;
    log("Y-matrica gotova.");
    if (stopRequested) { fail("Zaustavljeno."); return; }

    // injektirane snage
    std::vector<double> Pinj(n, 0.0), Qinj(n, 0.0);
    for (auto& [bid, gens] : genByBus)
        for (auto& g : gens) {
            auto itB = busIdToIdx.find(bid);
            if (itB == busIdToIdx.end()) continue;
            int idx = itB->second;
            Pinj[idx] += rowGet(g, 1) / baseMVA;
            Qinj[idx] += rowGet(g, 2) / baseMVA;
        }
    if (!opts.zeroLoads)
        for (int i = 0; i < n; ++i) {
            Pinj[i] -= rowGet(busMat[i], 2) / baseMVA;
            Qinj[i] -= rowGet(busMat[i], 3) / baseMVA;
        }
    progressPercent = 50;

    // Slack parametri
    int    slackId = slackNodes[0];
    int    slackIdx = busIdToIdx.count(slackId) ? busIdToIdx[slackId] : 0;
    double vSlack = genByBus.count(slackId) ? rowGet(genByBus[slackId][0], 5, 1.0) : rowGet(busMat[slackIdx], 7, 1.0);

    // Labele varijabli (kao u Python skriptu)
    const std::string V = "V";
    const std::string D = "delta";
    const std::string YM = "Y";
    const std::string YA = "phiY";

    // Pisanje .dmodl file-a
    std::ofstream out(outputFile.c_str());
    if (!out.is_open()) { fail("GRESKA: Ne mogu kreirati izlazni fajl."); return; }

    auto e = [&](const std::string& s) { out << s; };

    e("Header:\n\tmaxIter=100\n\treport=Solved\n\tmaxReps = -1\n\toutToTxt = false\nend\n");
    e("//Generisano MATPOWER Power Flow Converterom (polarne koordinate)\n");
    e("Model [type=NL domain=real eps=1e-6 name=\"PF in polar coordinates\"]:\n");

    log("Pisanje Vars...");
    progressPercent = 55;

    // varijable 
    e("Vars [out=true]:\n");
    for (int i = 0; i < n; ++i)
    {
        int bid = idxToId[i];
        bool isSlack = false;
        for (int s : slackNodes) if (s == bid) { isSlack = true; break; }
        if (isSlack) continue;

        double Vm = rowGet(busMat[i], 7, 1.0);
        double VaDeg = rowGet(busMat[i], 8);

        if (opts.flatStart) {
            e("\t" + D + "_" + std::to_string(bid) + " = " + D + "_" + std::to_string(slackId) + "; ");
            e(V + "_" + std::to_string(bid) + " = " + V + "_" + std::to_string(slackId) + "\n");
        }
        else {
            double vaRad = VaDeg * M_PI / 180.0;
            e("\t" + D + "_" + std::to_string(bid) + " = " + dbl(vaRad) + "; ");
            e(V + "_" + std::to_string(bid) + " = " + dbl(Vm) + "\n");
        }
    }

    log("Pisanje Params...");
    progressPercent = 60;

    // parametri
    e("Params:\n");

    for (int s : slackNodes) {
        int si = busIdToIdx.count(s) ? busIdToIdx[s] : 0;
        double vs = genByBus.count(s) ? rowGet(genByBus[s][0], 5, 1.0) : rowGet(busMat[si], 7, 1.0);
        double va = rowGet(busMat[si], 8) * M_PI / 180.0;
        e("\t" + D + "_" + std::to_string(s) + " = " + dbl(va) + " [out=true]; ");
        e(V + "_" + std::to_string(s) + " = " + dbl(vs) + " [out=true]\n");
    }

    for (int i = 0; i < n; ++i)
    {
        int bid = idxToId[i];
        Cmplx yii = Y_ii[i];
        double mag = std::abs(yii), ang = std::arg(yii);
        e("\t" + YM + "_" + std::to_string(bid) + "_" + std::to_string(bid) + " = " + dbl(mag));
        if (ang != 0.0)
            e("; " + YA + "_" + std::to_string(bid) + "_" + std::to_string(bid) + " = " + dbl(ang));
        e("\n");

        for (int j : nodeBranches[i]) {
            int bj = idxToId[j];
            auto it = Y_ij.find(yijKey(i, j));
            if (it == Y_ij.end()) continue;
            double magij = std::abs(it->second), angij = std::arg(it->second);
            e("\t" + YM + "_" + std::to_string(bid) + "_" + std::to_string(bj) + " = " + dbl(magij));
            if (angij != 0.0)
                e("; " + YA + "_" + std::to_string(bid) + "_" + std::to_string(bj) + " = " + dbl(angij));
            e("\n");
        }
    }

    for (int i = 0; i < n; ++i)
    {
        int bid = idxToId[i];
        bool isPV = false; for (int b : pvNodes) if (b == bid) { isPV = true;break; }
        bool isPQ = false; for (int b : pqNodes) if (b == bid) { isPQ = true;break; }

        if (isPV) {
            e("\tP_" + std::to_string(bid) + "_g = " + dbl(Pinj[i]) + "\n");
            if (opts.includeLimits)
                e("\tQ_" + std::to_string(bid) + "_g = " + dbl(Qinj[i]) + "\t[out=true]\n");
        }
        else if (isPQ && (Pinj[i] != 0.0 || Qinj[i] != 0.0)) {
            e("\tP_" + std::to_string(bid) + " = " + dbl(Pinj[i]) + "\n");
            e("\tQ_" + std::to_string(bid) + " = " + dbl(Qinj[i]) + "\n");
        }
    }

    for (int bid : pvNodes) {
        if (!genByBus.count(bid)) continue;
        double vsp = rowGet(genByBus[bid][0], 5, 1.0);
        e("\tV_" + std::to_string(bid) + "_sp = " + dbl(vsp) + "\n");
        if (opts.includeLimits) {
            double qmin = genByBus[bid][0][4] / baseMVA;
            double qmax = genByBus[bid][0][3] / baseMVA;
            e("\tcGen" + std::to_string(bid) + "Reg=true\n");
            e("\tQ_" + std::to_string(bid) + "_g_min = " + dbl(qmin) + "\n");
            e("\tQ_" + std::to_string(bid) + "_g_max = " + dbl(qmax) + "\n");
        }
    }

    log("Pisanje NLEs...");
    progressPercent = 70;
    if (stopRequested) { fail("Zaustavljeno."); return; }

    // NLEs (polarne jednacine balansa snage)
    e("NLEs:\n");

    for (int i = 0; i < n; ++i)
    {
        int bid = idxToId[i];
        bool isSlack = false; for (int s : slackNodes) if (s == bid) { isSlack = true;break; }
        if (isSlack) continue;

        bool isPV = false; for (int b : pvNodes) if (b == bid) { isPV = true;break; }
        const auto& adj = nodeBranches[i];

        // P jednacina
        {
            std::string lhs = "\t" + YM + "_" + std::to_string(bid) + "_" + std::to_string(bid) +
                "*" + V + "_" + std::to_string(bid) + "^2*cos(" +
                YA + "_" + std::to_string(bid) + "_" + std::to_string(bid) + ")";
            lhs += " + " + V + "_" + std::to_string(bid) + " * (";
            bool first = true;
            for (int j : adj) {
                int bj = idxToId[j];
                std::string term = YM + "_" + std::to_string(bid) + "_" + std::to_string(bj) +
                    "*" + V + "_" + std::to_string(bj) +
                    "*cos(" + D + "_" + std::to_string(bid) +
                    "-" + YA + "_" + std::to_string(bid) + "_" + std::to_string(bj) +
                    "-" + D + "_" + std::to_string(bj) + ")";
                lhs += first ? term : " + " + term;
                first = false;
            }
            if (!isPV) {
                if (Pinj[i] == 0.0 && Qinj[i] == 0.0) e(lhs + ") = 0\n");
                else e(lhs + ") = P_" + std::to_string(bid) + "\n");
            }
            else {
                if (Pinj[i] == 0.0) e(lhs + ") = 0\n");
                else e(lhs + ") = P_" + std::to_string(bid) + "_g\n");
            }
        }

        // Q jednačina-V constraint
        if (!isPV)
        {
            std::string lhs = "\t-" + YM + "_" + std::to_string(bid) + "_" + std::to_string(bid) +
                "*" + V + "_" + std::to_string(bid) + "^2*sin(" +
                YA + "_" + std::to_string(bid) + "_" + std::to_string(bid) + ")";
            lhs += " + " + V + "_" + std::to_string(bid) + " * (";
            bool first = true;
            for (int j : adj) {
                int bj = idxToId[j];
                std::string term = YM + "_" + std::to_string(bid) + "_" + std::to_string(bj) +
                    "*" + V + "_" + std::to_string(bj) +
                    "*sin(" + D + "_" + std::to_string(bid) +
                    "-" + YA + "_" + std::to_string(bid) + "_" + std::to_string(bj) +
                    "-" + D + "_" + std::to_string(bj) + ")";
                lhs += first ? term : " + " + term;
                first = false;
            }
            if (Pinj[i] == 0.0 && Qinj[i] == 0.0) e(lhs + ") = 0\n");
            else e(lhs + ") = Q_" + std::to_string(bid) + "\n");
        }
        else
        {
            e("\t" + V + "_" + std::to_string(bid) + "=" + V + "_" + std::to_string(bid) + "_sp\n");
        }
    }

    e("end\n");
    out.close();

    progressPercent = 100;
    log("Konverzija uspjesno zavrsena!");

    // rezultati za GUI
    results.buses = nBuses;
    results.gens = nGens;
    results.branches = nBranches;
    results.pqBuses = (int)pqNodes.size();
    results.pvBuses = (int)pvNodes.size();
    results.slacks = (int)slackNodes.size();
    results.outputFile = outputFile;
    _lastResults = results;

    if (doneCb) doneCb(true, td::String("Konverzija uspjesno zavrsena!"), results);
}