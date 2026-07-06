#pragma once
#include <gui/StandardTabView.h>
#include "ViewInput.h"
#include "ViewProgress.h"
#include "ViewResults.h"

class MainView : public gui::StandardTabView
{
protected:
    ViewInput    _viewInput;
    ViewProgress _viewProgress;
    ViewResults  _viewResults;

public:
    MainView()
    {
        addView(&_viewInput,    tr("TabInput"));
        addView(&_viewProgress, tr("TabProgress"));
        addView(&_viewResults,  tr("TabResults"));
    }

    ViewInput&    getInputView()    { return _viewInput;    }
    ViewProgress& getProgressView() { return _viewProgress; }
    ViewResults&  getResultsView()  { return _viewResults;  }
};
