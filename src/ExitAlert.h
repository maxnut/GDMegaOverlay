#pragma once
#include "pch.h"

class ExitAlert : public gd::FLAlertLayerProtocol {
    protected:
        
        void FLAlert_Clicked(gd::FLAlertLayer *layer, bool btn2) override;
};
