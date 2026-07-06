#pragma once
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/LineEdit.h>
#include <gui/Button.h>
#include <gui/CheckBox.h>
#include <gui/GridLayout.h>
#include <gui/VerticalLayout.h>
#include <gui/HorizontalLayout.h>
#include <gui/GridComposer.h>
#include <gui/FileDialog.h>
#include <functional>
#include "Converter.h"

class ViewInput : public gui::View
{
private:
    std::function<void()> _onConvert;
    std::function<void()> _onClear;

protected:
    gui::Label    _lblInputFile;
    gui::LineEdit _edInputFile;
    gui::Button   _btnBrowseInput;

    gui::Label    _lblOutputFile;
    gui::LineEdit _edOutputFile;
    gui::Button   _btnBrowseOutput;

    gui::Label    _lblOptionsSection;
    gui::CheckBox _chkFlatStart;
    gui::CheckBox _chkIncludeLimits;
    gui::CheckBox _chkZeroLoads;

    gui::Button   _btnConvert;
    gui::Button   _btnClear;

    gui::GridLayout       _glFiles;
    gui::HorizontalLayout _hlOptions;
    gui::HorizontalLayout _hlButtons;
    gui::VerticalLayout   _vl;

public:
    ViewInput()
        : _lblInputFile(tr("InputFileLabel"))
        , _edInputFile()
        , _btnBrowseInput(tr("Browse"))
        , _lblOutputFile(tr("OutputFileLabel"))
        , _edOutputFile()
        , _btnBrowseOutput(tr("Browse"))
        , _lblOptionsSection(tr("OptionsSection"))
        , _chkFlatStart(tr("OptFlatStart"))
        , _chkIncludeLimits(tr("OptIncludeLimits"))
        , _chkZeroLoads(tr("OptZeroLoads"))
        , _btnConvert(tr("StartConversion"))
        , _btnClear(tr("ClearResults"))
        , _glFiles(2, 3)
        , _hlOptions(3)
        , _hlButtons(2)
        , _vl(6)
    {
        _chkFlatStart.setValue(true);
        _chkIncludeLimits.setValue(false);
        _chkZeroLoads.setValue(false);

        {
            gui::GridComposer gc(_glFiles);
            gc.appendRow(_lblInputFile);
            gc.appendCol(_edInputFile);
            gc.appendCol(_btnBrowseInput);
            gc.appendRow(_lblOutputFile);
            gc.appendCol(_edOutputFile);
            gc.appendCol(_btnBrowseOutput);
        }

        _hlOptions << _chkFlatStart << _chkIncludeLimits << _chkZeroLoads;

        _btnConvert.setType(gui::Button::Type::Default);
        _btnClear.setType(gui::Button::Type::Destructive);
        _hlButtons << _btnConvert << _btnClear;

        _vl << _glFiles << _lblOptionsSection << _hlOptions << _hlButtons;
        setLayout(&_vl);

        // Browse input – poziva se iz View 
        _btnBrowseInput.onClick([this]()
            {
                gui::OpenFileDialog::show(this, tr("SelectInputFile"), "*.m", 1000,
                    [this](gui::FileDialog* pDlg)
                    {
                        if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                        {
                            td::String path = pDlg->getFileName();
                            if (path.isEmpty()) return;
                            _edInputFile.setText(path);

                            // Autofill output putanju
                            std::string sp(path.c_str());
                            auto dot = sp.rfind('.');
                            if (dot != std::string::npos) sp = sp.substr(0, dot);
                            sp += ".dmodl";
                            _edOutputFile.setText(td::String(sp.c_str()));
                        }
                    });
            });

        // Browse output – poziva se iz View 
        _btnBrowseOutput.onClick([this]()
            {
                gui::OpenFileDialog::show(this, tr("SelectOutputFile"), "*.dmodl", 2000,
                    [this](gui::FileDialog* pDlg)
                    {
                        if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                        {
                            td::String path = pDlg->getFileName();
                            if (path.isEmpty()) return;
                            std::string sp(path.c_str());
                            if (sp.size() < 7 || sp.substr(sp.size() - 6) != ".dmodl")
                                sp += ".dmodl";
                            _edOutputFile.setText(td::String(sp.c_str()));
                        }
                    });
            });

        _btnConvert.onClick([this]() { if (_onConvert) _onConvert(); });
        _btnClear.onClick([this]() { if (_onClear)   _onClear();   });
    }

    void setOnConvert(std::function<void()> fn) { _onConvert = fn; }
    void setOnClear(std::function<void()> fn) { _onClear = fn; }

    td::String getInputFile()  const { return _edInputFile.getText(); }
    td::String getOutputFile() const { return _edOutputFile.getText(); }

    void setInputFile(const td::String& p) { _edInputFile.setText(p); }
    void setOutputFile(const td::String& p) { _edOutputFile.setText(p); }

    Converter::Options getOptions() const
    {
        Converter::Options o;
        o.flatStart = _chkFlatStart.isChecked();
        o.includeLimits = _chkIncludeLimits.isChecked();
        o.zeroLoads = _chkZeroLoads.isChecked();
        return o;
    }

    void clearInputs()
    {
        _edInputFile.setText(td::String(""));
        _edOutputFile.setText(td::String(""));
    }
};