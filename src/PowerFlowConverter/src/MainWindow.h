#pragma once
#include <gui/Window.h>
#include <gui/ActionItem.h>
#include <gui/Timer.h>
#include <gui/Thread.h>
#include "MenuBar.h"
#include "ToolBar.h"
#include "MainView.h"
#include "Converter.h"
#include <thread>
#include <atomic>

class MainWindow : public gui::Window
{
protected:
    MenuBar  _mainMenuBar;
    ToolBar  _toolBar;
    MainView _mainView;

    Converter   _converter;
    std::thread _workerThread;
    gui::Timer  _progressTimer;

public:
    MainWindow()
    : gui::Window(gui::Size(900, 560))
    , _progressTimer(this, 0.1f, false)
    {
        setTitle(tr("AppTitle"));
        _mainMenuBar.setAsMain(this, gui::MenuBar::Location::SystemSpecific);
        setToolBar(_toolBar);
        setCentralView(&_mainView);

        _mainView.getInputView().setOnConvert([this]() { startConversion(); });
        _mainView.getInputView().setOnClear([this]()   { clearAll();        });
        _mainView.getProgressView().setOnStop([this]() { stopConversion();  });

        _progressTimer.onTimer([this]()
        {
            int pct = _converter.progressPercent.load();
            _mainView.getProgressView().setProgress(pct / 100.0);
        });
    }

    ~MainWindow()
    {
        _converter.stopRequested = true;
        if (_workerThread.joinable()) _workerThread.join();
    }

    bool shouldClose() override { return true; }

    void onClose() override
    {
        _converter.stopRequested = true;
        if (_workerThread.joinable()) _workerThread.join();
        gui::Window::onClose();
    }

    bool onActionItem(gui::ActionItemDescriptor& aiDesc) override
    {
        auto [menuID, first, last, actionID] = aiDesc.getIDs();

        // Pokreni / Zaustavi / Obrisi (toolbar + meni)
        if (menuID == 20 && first == 0 && last == 0)
        {
            if (actionID == 10) { startConversion(); return true; }
            if (actionID == 20) { stopConversion();  return true; }
            if (actionID == 30) { clearAll();        return true; }
        }

        // O aplikaciji (toolbar + app meni)
        if (menuID == 10 && actionID == 10)
        {
            showAlert(tr("AppTitle"), tr("AboutText"));
            return true;
        }
        return false;
    }

private:
    void startConversion()
    {
        if (_workerThread.joinable())
        {
            showAlert(tr("AppTitle"), tr("AlreadyRunning"));
            return;
        }

        td::String inputFile  = _mainView.getInputView().getInputFile();
        td::String outputFile = _mainView.getInputView().getOutputFile();

        if (inputFile.length() == 0 || outputFile.length() == 0)
        {
            showAlert(tr("AppTitle"), tr("NoFilesSelected"));
            return;
        }

        Converter::Options opts = _mainView.getInputView().getOptions();

        _mainView.getProgressView().clearLog();
        _mainView.getProgressView().appendLog(tr("ConversionStarted"));
        _mainView.setCurrentViewPos(1);

        _converter.stopRequested   = false;
        _converter.progressPercent = 0;

        _progressTimer.start();

        ViewProgress* pProgView  = &_mainView.getProgressView();
        gui::Timer*   pTimer     = &_progressTimer;
        MainView*     pMainView  = &_mainView;
        Converter*    pConverter = &_converter;

        _workerThread = std::thread([this, inputFile, outputFile, opts,
                                     pProgView, pTimer, pMainView, pConverter]()
        {
            try
            {
                pConverter->convert(
                    inputFile,
                    outputFile,
                    opts,

                    [pProgView](const td::String& line)
                    {
                        td::String lineCopy = line;
                        gui::thread::asyncExecInMainThread([pProgView, lineCopy]()
                        {
                            pProgView->appendLog(lineCopy);
                        });
                    },

                    [this, pProgView, pTimer, pMainView, pConverter](
                        bool ok, const td::String& msg, const Converter::Results& res)
                    {
                        bool               okCopy  = ok;
                        td::String         msgCopy = msg;
                        Converter::Results resCopy = res;
                        gui::thread::asyncExecInMainThread([this, okCopy, msgCopy, resCopy,
                                                            pProgView, pTimer, pMainView, pConverter]()
                        {
                            pTimer->stop();
                            pProgView->setProgress(pConverter->progressPercent.load() / 100.0);
                            pProgView->appendLog(msgCopy);

                            if (okCopy)
                            {
                                pMainView->getResultsView().setResults(
                                    resCopy.buses,
                                    resCopy.gens,
                                    resCopy.branches,
                                    resCopy.pqBuses,
                                    resCopy.pvBuses,
                                    resCopy.slacks,
                                    resCopy.outputFile);
                            }

                            if (_workerThread.joinable())
                                _workerThread.detach();
                        });
                    }
                );
            }
            catch (const std::exception& ex)
            {
                td::String errMsg;
                errMsg.format("GRESKA: %s", ex.what());
                gui::thread::asyncExecInMainThread([pProgView, errMsg]()
                {
                    pProgView->appendLog(errMsg);
                });
            }
            catch (...)
            {
                gui::thread::asyncExecInMainThread([pProgView]()
                {
                    pProgView->appendLog(td::String("GRESKA: nepoznata greska u konverziji"));
                });
            }
        });
    }

    void stopConversion()
    {
        _converter.stopRequested = true;
        _mainView.getProgressView().appendLog(tr("ConversionStopped"));
    }

    void clearAll()
    {
        if (_workerThread.joinable())
        {
            showAlert(tr("AppTitle"), tr("AlreadyRunning"));
            return;
        }
        _mainView.getProgressView().clearLog();
        _mainView.getResultsView().clearResults();
        _mainView.getInputView().clearInputs();
        _converter.progressPercent = 0;
    }
};
