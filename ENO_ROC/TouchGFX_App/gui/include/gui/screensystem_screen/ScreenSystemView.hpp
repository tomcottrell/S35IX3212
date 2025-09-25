#ifndef SCREENSYSTEMVIEW_HPP
#define SCREENSYSTEMVIEW_HPP

#include <gui_generated/screensystem_screen/ScreenSystemViewBase.hpp>
#include <gui/screensystem_screen/ScreenSystemPresenter.hpp>

class ScreenSystemView : public ScreenSystemViewBase
{
public:
    ScreenSystemView();
    virtual ~ScreenSystemView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // SCREENSYSTEMVIEW_HPP
