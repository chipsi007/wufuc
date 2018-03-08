#pragma once

HANDLE event_create_with_string_security_descriptor(
        bool ManualReset,
        bool InitialState,
        const wchar_t *Name,
        const wchar_t *StringSecurityDescriptor);
