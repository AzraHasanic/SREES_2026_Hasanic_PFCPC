#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/ProgressIndicator.h>
#include <gui/TextEdit.h>
#include <gui/Button.h>
#include <gui/HorizontalLayout.h>
#include <gui/VerticalLayout.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <functional>

class ViewProgress : public gui::View
{
private:
    std::function<void()> _onStop;

protected:
    gui::Label             _lblProgressTitle;
    gui::Label             _lblProgressValue;
    gui::ProgressIndicator _progressBar;
    gui::Label             _lblLogTitle;
    gui::TextEdit          _logEdit;
    gui::Button            _btnStop;
    gui::GridLayout        _glProgress;
    gui::HorizontalLayout  _hlStop;
    gui::VerticalLayout    _vl;

public:
    ViewProgress()
    : _lblProgressTitle("Napredak konverzije:")
    , _lblProgressValue("0 %")
    , _progressBar(gui::DataCtrl::Orientation::Horizontal)
    , _lblLogTitle("Log konverzije:")
    , _btnStop("Zaustavi")
    , _glProgress(1, 3)
    , _hlStop(1)
    , _vl(5)
    {
        {
            gui::GridComposer gc(_glProgress);
            gc.appendRow(_lblProgressTitle);
            gc.appendCol(_progressBar);
            gc.appendCol(_lblProgressValue);
        }

        _progressBar.setValue(0.0);
        _logEdit.setAsReadOnly();
        _btnStop.setType(gui::Button::Type::Destructive);
        _hlStop << _btnStop;

        _vl << _glProgress << _hlStop << _lblLogTitle << _logEdit;
        setLayout(&_vl);

        _btnStop.onClick([this]() { if (_onStop) _onStop(); });
    }

    void setOnStop(std::function<void()> fn) { _onStop = fn; }

    void setProgress(double value)
    {
        _progressBar.setValue(value);
        td::String s;
        s.format("%.0f %%", value * 100.0);
        _lblProgressValue.setTitle(s);
    }

    void appendLog(const td::String& line)
    {
        td::String l = line;
        l += "\n";
        _logEdit.appendString(l);
    }

    void clearLog()
    {
        _logEdit.clean();
        setProgress(0.0);
    }
};
