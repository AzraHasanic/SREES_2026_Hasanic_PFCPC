#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/Button.h>
#include <gui/LineEdit.h>
#include <gui/CheckBox.h>
#include <gui/HorizontalLayout.h>
#include <gui/VerticalLayout.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <gui/FileDialog.h>
#include "PowerFlowPlugin.h"
#include "Converter.h"
#include <functional>

class ViewConv : public gui::View
{
private:
    std::function<void()> _onConvert;
    std::function<void()> _onStop;

protected:
    gui::Label    _lblFnIn;
    gui::LineEdit _editFnIn;
    gui::Button   _btnSelectInFn;

    gui::Label    _lblFnOut;
    gui::LineEdit _editFnOut;
    gui::Button   _btnSelectOutFn;

    gui::Label    _lblOptions;
    gui::CheckBox _chkFlatStart;
    gui::CheckBox _chkIncludeLimits;
    gui::CheckBox _chkZeroLoads;

    gui::Button   _btnConvert;
    gui::Button   _btnStop;

    gui::GridLayout       _glFiles;
    gui::HorizontalLayout _hlOptions;
    gui::HorizontalLayout _hlButtons;
    gui::VerticalLayout   _vl;

    td::UINT4 _wndID;

public:
    ViewConv(td::UINT4 wndID = 0)
    : _lblFnIn("Ulazna datoteka (.m):")
    , _btnSelectInFn("...")
    , _lblFnOut("Izlazna datoteka (.dmodl):")
    , _btnSelectOutFn("...")
    , _lblOptions("Opcije konverzije:")
    , _chkFlatStart("Flat start")
    , _chkIncludeLimits("Q-ogranicenja PV cvorova")
    , _chkZeroLoads("Nuliraj potrosnje (test)")
    , _btnConvert("Pokreni konverziju")
    , _btnStop("Zaustavi")
    , _glFiles(2, 3)
    , _hlOptions(3)
    , _hlButtons(2)
    , _vl(5)
    , _wndID(wndID)
    {
        _chkFlatStart.setValue(true);
        _chkIncludeLimits.setValue(false);
        _chkZeroLoads.setValue(false);
        _btnConvert.setType(gui::Button::Type::Default);
        _btnStop.setType(gui::Button::Type::Destructive);

        {
            gui::GridComposer gc(_glFiles);
            gc.appendRow(_lblFnIn);
            gc.appendCol(_editFnIn);
            gc.appendCol(_btnSelectInFn);
            gc.appendRow(_lblFnOut);
            gc.appendCol(_editFnOut);
            gc.appendCol(_btnSelectOutFn);
        }

        _hlOptions << _chkFlatStart << _chkIncludeLimits << _chkZeroLoads;
        _hlButtons << _btnConvert << _btnStop;

        _vl << _glFiles << _lblOptions << _hlOptions << _hlButtons;
        setLayout(&_vl);

        _btnSelectInFn.onClick([this]()
        {
            gui::OpenFileDialog::show(this, "Odaberi MATPOWER ulaznu datoteku", "*.m",
                _wndID + 1000, [this](gui::FileDialog* pDlg)
                {
                    if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                    {
                        td::String path = pDlg->getFileName();
                        if (path.isEmpty()) return;
                        _editFnIn = path;
                        std::string sp(path.c_str());
                        auto dot = sp.rfind('.');
                        if (dot != std::string::npos) sp = sp.substr(0, dot);
                        sp += ".dmodl";
                        _editFnOut = td::String(sp.c_str());
                    }
                });
        });

        _btnSelectOutFn.onClick([this]()
        {
            gui::SaveFileDialog::show(this, "Snimi izlaznu .dmodl datoteku", "*.dmodl",
                _wndID + 2000, [this](gui::FileDialog* pDlg)
                {
                    if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                    {
                        td::String path = pDlg->getFileName();
                        if (path.isEmpty()) return;
                        std::string sp(path.c_str());
                        if (sp.size() < 7 || sp.substr(sp.size()-6) != ".dmodl")
                            sp += ".dmodl";
                        _editFnOut = td::String(sp.c_str());
                    }
                });
        });

        _btnConvert.onClick([this]() { if (_onConvert) _onConvert(); });
        _btnStop.onClick([this]()    { if (_onStop)    _onStop();    });
    }

    void setOnConvert(std::function<void()> fn) { _onConvert = fn; }
    void setOnStop   (std::function<void()> fn) { _onStop    = fn; }

    td::String getInputFile()  const { return _editFnIn.getText();  }
    td::String getOutputFile() const { return _editFnOut.getText(); }

    Converter::Options getOptions() const
    {
        Converter::Options o;
        o.flatStart     = _chkFlatStart.isChecked();
        o.includeLimits = _chkIncludeLimits.isChecked();
        o.zeroLoads     = _chkZeroLoads.isChecked();
        return o;
    }
};
