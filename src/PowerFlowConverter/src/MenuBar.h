#pragma once
#include <gui/MenuBar.h>

class MenuBar : public gui::MenuBar
{
private:
    gui::SubMenu _subApp;
    gui::SubMenu _subConvert;

    void populateSubAppMenu()
    {
        auto& items = _subApp.getItems();
        items[0].initAsActionItem(tr("About"), 10, "a");
        items[1].initAsQuitAppActionItem(tr("Quit"), "q");
    }

    void populateSubConvertMenu()
    {
        auto& items = _subConvert.getItems();
        items[0].initAsActionItem(tr("StartConversion"), 10, "r");
        items[1].initAsActionItem(tr("StopConversion"),  20, "t");
        items[2].initAsActionItem(tr("ClearResults"),    30, "l");
    }

public:
    MenuBar()
    : gui::MenuBar(2)
    , _subApp(10, "App", 2)
    , _subConvert(20, tr("ConvertMenu"), 3)
    {
        populateSubAppMenu();
        populateSubConvertMenu();
        _menus[0] = &_subApp;
        _menus[1] = &_subConvert;
    }
};
