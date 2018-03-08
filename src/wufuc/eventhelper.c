#include "stdafx.h"
#include "eventhelper.h"

#include <sddl.h>

HANDLE event_create_with_string_security_descriptor(
        bool ManualReset,
        bool InitialState,
        const wchar_t *Name,
        const wchar_t *StringSecurityDescriptor)
{
        SECURITY_ATTRIBUTES sa = { sizeof sa };

        if ( ConvertStringSecurityDescriptorToSecurityDescriptorW(
                StringSecurityDescriptor,
                SDDL_REVISION_1,
                &sa.lpSecurityDescriptor,
                NULL) ) {

                return CreateEventW(&sa, ManualReset, InitialState, Name);
        }
        return NULL;
}
