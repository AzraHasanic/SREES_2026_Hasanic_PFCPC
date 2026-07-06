#pragma once
#include <gui/ToolBar.h>
#include <gui/Image.h>

class ToolBar : public gui::ToolBar
{
protected:
    gui::Image _imgStart;
    gui::Image _imgStop;
    gui::Image _imgAbout;

public:
    ToolBar()
    : gui::ToolBar("pfcToolBar", 3)
    , _imgStart(":start")
    , _imgStop(":stop")
    , _imgAbout(":clear")
    {
        addItem(td::String(""), &_imgStart, tr("StartConversionTT"), 20, 0, 0, 10);
        addItem(td::String(""), &_imgStop,  tr("StopConversionTT"),  20, 0, 0, 20);
        addItem(td::String(""), &_imgAbout, tr("AboutTT"),           10, 0, 0, 10);
    }
};
