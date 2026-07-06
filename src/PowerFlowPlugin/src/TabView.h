#pragma once
#include <gui/StandardTabView.h>
#include <gui/Timer.h>
#include <gui/Thread.h>
#include "ViewConv.h"
#include "ViewProgress.h"
#include <thread>

class TabView : public gui::StandardTabView
{
protected:
    ViewConv     _viewConv;
    ViewProgress _viewProgress;

    sc::IPlugin*             _pIPlugin;
    sc::IPlugin::CallBack    _onComplete;
    Converter                _converter;
    std::thread              _workerThread;
    gui::Timer               _progressTimer;

public:
    TabView(sc::IPlugin* pIPlugin, const sc::IPlugin::CallBack& onComplete, td::UINT4 wndID)
    : _pIPlugin(pIPlugin)
    , _onComplete(onComplete)
    , _progressTimer(this, 0.1f, false)
    {
        addView(&_viewConv,     "Konvertor");
        addView(&_viewProgress, "Tok konverzije");

        // Progress timer - 100ms
        _progressTimer.onTimer([this]()
        {
            int pct = _converter.progressPercent.load();
            _viewProgress.setProgress(pct / 100.0);
        });

        // konverzija iz ViewConv
        _viewConv.setOnConvert([this]()
        {
            if (_workerThread.joinable()) return;

            td::String inputFile  = _viewConv.getInputFile();
            td::String outputFile = _viewConv.getOutputFile();
            if (inputFile.isEmpty() || outputFile.isEmpty()) return;

            Converter::Options opts = _viewConv.getOptions();

            _viewProgress.clearLog();
            _viewProgress.appendLog("Pokretanje konverzije...");
            setCurrentViewPos(1);   // prebaci na Progress tab

            _converter.stopRequested   = false;
            _converter.progressPercent = 0;
            _progressTimer.start();

            ViewProgress* pProg      = &_viewProgress;
            TabView*      pThis      = this;
            Converter*    pConverter = &_converter;
            gui::Timer*   pTimer     = &_progressTimer;

            _workerThread = std::thread([pThis, inputFile, outputFile, opts,
                                         pProg, pConverter, pTimer]()
            {
                pConverter->convert(
                    inputFile, outputFile, opts,

                    [pProg](const td::String& line)
                    {
                        td::String lineCopy = line;
                        gui::thread::asyncExecInMainThread([pProg, lineCopy]()
                        {
                            pProg->appendLog(lineCopy);
                        });
                    },

                    [pThis, pProg, pConverter, pTimer](
                        bool ok, const td::String& msg, const Converter::Results& res)
                    {
                        bool       okCopy  = ok;
                        td::String msgCopy = msg;
                        gui::thread::asyncExecInMainThread([pThis, okCopy, msgCopy,
                                                            pProg, pConverter, pTimer]()
                        {
                            pTimer->stop();
                            pProg->setProgress(pConverter->progressPercent.load() / 100.0);
                            pProg->appendLog(msgCopy);

                            if (okCopy && pThis->_onComplete && pThis->_pIPlugin)
                                pThis->_onComplete(pThis->_pIPlugin);

                            if (pThis->_workerThread.joinable())
                                pThis->_workerThread.detach();
                        });
                    }
                );
            });
        });

        _viewConv.setOnStop([this]()
        {
            _converter.stopRequested = true;
            _viewProgress.appendLog("Konverzija zaustavljena.");
        });

        _viewProgress.setOnStop([this]()
        {
            _converter.stopRequested = true;
            _viewProgress.appendLog("Konverzija zaustavljena.");
        });
    }

    ~TabView()
    {
        _converter.stopRequested = true;
        if (_workerThread.joinable()) _workerThread.join();
    }

    td::String getOutFileName() const
    {
        return _viewConv.getOutputFile();
    }
};
