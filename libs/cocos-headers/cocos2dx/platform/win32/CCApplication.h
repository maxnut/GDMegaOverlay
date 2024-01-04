#ifndef __CC_APPLICATION_WIN32_H__
#define __CC_APPLICATION_WIN32_H__

#include "ccMacros.h"
#include "CCStdC.h"
#include "platform/CCCommon.h"
#include "platform/CCApplicationProtocol.h"
#include <string>
#include <Xinput.h>

NS_CC_BEGIN

class CC_DLL CCControllerHandler
{
public:
    //CCControllerHandler();
    //CCControllerState getState();
    //void updateConnected();

    XINPUT_STATE m_xinputState;
    int m_userIndex;
    bool m_controllerConnected;
    bool m_dpadUp;
    bool m_dpadDown;
    bool m_dpadLeft;
    bool m_dpadRight;
    bool m_start;
    bool m_back;
    bool m_lt;
    bool m_rt;
    bool m_lb;
    bool m_rb;
    bool m_buttonA;
    bool m_buttonB;
    bool m_buttonX;
    bool m_buttonY;
};

class CCRect;

class CC_DLL CCApplication : public CCApplicationProtocol
{
public:
    CCApplication();
    virtual ~CCApplication();

    /**
    @brief    Run the message loop.
    */
    RT_REMOVE(virtual int run(); )
        RT_ADD(virtual void gameDidSave(); )

        /**
        @brief    Get current applicaiton instance.
        @return Current application instance pointer.
        */
        static CCApplication* sharedApplication();

    /* override functions */
    virtual void setAnimationInterval(double interval);
    virtual ccLanguageType getCurrentLanguage();

    /**
     @brief Get target platform
     */
    virtual TargetPlatform getTargetPlatform();

    virtual void openURL(const char* url);
    virtual int run();
    virtual void setupGLView();
    virtual void platformShutdown();
    void toggleVerticalSync(bool);
    RT_ADD(
        void setupVerticalSync();
        void updateVerticalSync();
        void updateControllerKeys();

        int getTimeElapsed();
        void resetForceTimer();

        void leftMouseDown();
        void leftMouseUp();

        void logTimeElapsed(std::string);

        void moveMouse(int, int);

        void shutdownApplication();
        void toggleMouseControl(bool);
        void updateController();
        void updateMouseControl();

    )

        /**
         *  Sets the Resource root path.
         *  @deprecated Please use CCFileUtils::sharedFileUtils()->setSearchPaths() instead.
         */
        CC_DEPRECATED_ATTRIBUTE void setResourceRootPath(const std::string& rootResDir);

    /**
     *  Gets the Resource root path.
     *  @deprecated Please use CCFileUtils::sharedFileUtils()->getSearchPaths() instead.
     */
    CC_DEPRECATED_ATTRIBUTE const std::string& getResourceRootPath(void);

    void setStartupScriptFilename(const std::string& startupScriptFile);

    const std::string& getStartupScriptFilename(void)
    {
        return m_startupScriptFilename;
    }

public:
    HINSTANCE           m_hInstance;
    HACCEL              m_hAccelTable;
    LARGE_INTEGER       m_nAnimationInterval;
    LARGE_INTEGER       m_nVsyncInterval;
    std::string          m_resourceRootPath;
    std::string          m_startupScriptFilename;
    CCControllerHandler* m_pControllerHandler;
    bool m_bUpdateController;
    CC_SYNTHESIZE_NV(bool, m_bShutdownCalled, ShutdownCalled);
    INPUT m_iInput;
    CCPoint m_obLeftThumb;
    CCPoint m_obRightThumb;
    bool m_bMouseControl;
    float m_fAnimationInterval;
    float m_fVsyncInterval;
    CC_SYNTHESIZE_READONLY_NV(bool, m_bVerticalSyncEnabled, VerticalSyncEnabled);
    CC_SYNTHESIZE_READONLY_NV(bool, m_bControllerConnected, ControllerConnected);
    CC_SYNTHESIZE_NV(bool, m_bSleepMode, SleepMode);
    CC_SYNTHESIZE_NV(bool, m_bForceTimer, ForceTimer);
    CC_SYNTHESIZE_NV(bool, m_bSmoothFix, SmoothFix);
    CC_SYNTHESIZE_NV(bool, m_bFullscreen, Fullscreen);

    static CCApplication* sm_pSharedApplication;
};

NS_CC_END

#endif    // __CC_APPLICATION_WIN32_H__
