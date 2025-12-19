#pragma once

#include <v8.h>

// #include "IdaTemplate.h"

namespace Ida
{
    void initTemplates(v8::Isolate *isolate/*, add bridges to communicate with external world here */);
    
    // TODO - add custom templates here
    // IdaTemplate *getIdaTemplate();

    void deleteTemplates();
}  // namespace Ida
