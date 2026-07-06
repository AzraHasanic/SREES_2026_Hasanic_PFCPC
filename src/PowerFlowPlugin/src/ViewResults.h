#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/NumericEdit.h>
#include <gui/LineEdit.h>
#include <gui/Button.h>
#include <gui/GridLayout.h>
#include <gui/VerticalLayout.h>
#include <gui/HorizontalLayout.h>
#include <gui/GridComposer.h>

class ViewResults : public gui::View
{
protected:
    gui::Label       _lblNetworkSection;
    gui::Label       _lblBuses;
    gui::NumericEdit _edBuses;
    gui::Label       _lblGens;
    gui::NumericEdit _edGens;
    gui::Label       _lblBranches;
    gui::NumericEdit _edBranches;
    gui::Label       _lblPQBuses;
    gui::NumericEdit _edPQBuses;
    gui::Label       _lblPVBuses;
    gui::NumericEdit _edPVBuses;
    gui::Label       _lblSlacks;
    gui::NumericEdit _edSlacks;

    gui::Label    _lblOutputSection;
    gui::Label    _lblOutputPath;
    gui::LineEdit _edOutputPath;

    gui::GridLayout       _glStats;
    gui::VerticalLayout   _vl;

public:
    ViewResults()
        : _lblNetworkSection(tr("NetworkSection"))
        , _lblBuses(tr("ResBuses"))
        , _edBuses(td::int4)
        , _lblGens(tr("ResGens"))
        , _edGens(td::int4)
        , _lblBranches(tr("ResBranches"))
        , _edBranches(td::int4)
        , _lblPQBuses(tr("ResPQBuses"))
        , _edPQBuses(td::int4)
        , _lblPVBuses(tr("ResPVBuses"))
        , _edPVBuses(td::int4)
        , _lblSlacks(tr("ResSlacks"))
        , _edSlacks(td::int4)
        , _lblOutputSection(tr("OutputSection"))
        , _lblOutputPath(tr("ResOutputPath"))
        , _glStats(6, 2)
        , _vl(5)
    {
        _edBuses.setAsReadOnly();
        _edGens.setAsReadOnly();
        _edBranches.setAsReadOnly();
        _edPQBuses.setAsReadOnly();
        _edPVBuses.setAsReadOnly();
        _edSlacks.setAsReadOnly();
        _edOutputPath.setAsReadOnly();

        {
            gui::GridComposer gc(_glStats);
            gc.appendRow(_lblBuses);    gc.appendCol(_edBuses);
            gc.appendRow(_lblGens);     gc.appendCol(_edGens);
            gc.appendRow(_lblBranches); gc.appendCol(_edBranches);
            gc.appendRow(_lblPQBuses);  gc.appendCol(_edPQBuses);
            gc.appendRow(_lblPVBuses);  gc.appendCol(_edPVBuses);
            gc.appendRow(_lblSlacks);   gc.appendCol(_edSlacks);
        }

        _vl << _lblNetworkSection << _glStats
            << _lblOutputSection << _lblOutputPath << _edOutputPath;
        setLayout(&_vl);
    }

    void setResults(int buses, int gens, int branches,
        int pq, int pv, int slacks,
        const td::String& outPath)
    {
        td::Variant v;
        v = buses;    _edBuses.setValue(v);
        v = gens;     _edGens.setValue(v);
        v = branches; _edBranches.setValue(v);
        v = pq;       _edPQBuses.setValue(v);
        v = pv;       _edPVBuses.setValue(v);
        v = slacks;   _edSlacks.setValue(v);
        _edOutputPath.setText(outPath);
    }

    void clearResults()
    {
        td::Variant v(0);
        _edBuses.setValue(v);
        _edGens.setValue(v);
        _edBranches.setValue(v);
        _edPQBuses.setValue(v);
        _edPVBuses.setValue(v);
        _edSlacks.setValue(v);
        _edOutputPath.setText(td::String(""));
    }
};