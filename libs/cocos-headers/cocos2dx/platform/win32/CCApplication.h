#ifndef __CC_APPLICATION_WIN32_H__
#define __CC_APPLICATION_WIN32_H__

#include "ccMacros.h"
#include "CCStdC.h"
#include "platform/CCCommon.h"
#include "platform/CCApplicationProtocol.h"
#include <string>

NS_CC_BEGIN

class CCRect;

class CC_DLL CCApplication : public CCApplicationProtocol
{
public:
    CCApplication();
    virtual ~CCApplication();

    /**
    @brief    Run the message loop.
    */
    RT_REMOVE(  virtual int run();  )
    RT_ADD(     virtual void gameDidSave(); )

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

    RT_ADD(
        virtual void openURL(const char* url);
        virtual int run();
        virtual void setupGLView();
        virtual void platformShutdown();
        void toggleVerticalSync(bool);
        bool getVerticalSyncEnabled() const;
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

    bool getControllerConnected() const;

    const std::string& getStartupScriptFilename(void)
    {
        return m_startupScriptFilename;
    }

public:
    HINSTANCE           m_hInstance;
    HACCEL              m_hAccelTable;
    LARGE_INTEGER       m_nAnimationInterval;
    std::string         m_resourceRootPath;
    std::string         m_startupScriptFilename;

    static CCApplication * sm_pSharedApplication;
};

NS_CC_END

#endif    // __CC_APPLICATION_WIN32_H__
