/*
 * Copyright (c) 2016-present Samsung Electronics Co., Ltd
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef __EscargotStringBuilder__
#define __EscargotStringBuilder__

#include "runtime/String.h"
#include "util/Vector.h"

namespace Escargot {

class ExecutionState;

class StringBuilder {
    MAKE_STACK_ALLOCATED();
    struct StringBuilderPiece {
        String* m_string;
        size_t m_start, m_end;
    };

    void appendPiece(String* str, size_t s, size_t e);

public:
    StringBuilder()
    {
        m_has8BitContent = true;
        m_contentLength = 0;
        m_piecesInlineStorageUsage = 0;
    }

    size_t contentLength() { return m_contentLength; }
    void appendString(const char* str)
    {
        appendPiece(new ASCIIString(str), 0, strlen(str));
    }

    void appendChar(char16_t ch)
    {
        char16_t s = ch;
        appendString(new UTF16String(&s, 1));
    }

    void appendChar(char ch)
    {
        char s = ch;
        appendString(new ASCIIString(&s, 1));
    }

    void appendString(String* str)
    {
        appendPiece(str, 0, str->length());
    }

    void appendSubString(String* str, size_t s, size_t e)
    {
        appendPiece(str, s, e);
    }

    String* finalize(ExecutionState* state = nullptr); // provide ExecutionState if you need limit of string length(exception can be thrown only in ExecutionState area)

protected:
    bool m_has8BitContent;
    size_t m_piecesInlineStorageUsage;
    size_t m_contentLength;
    StringBuilderPiece m_piecesInlineStorage[STRING_BUILDER_INLINE_STORAGE_MAX];
    Vector<StringBuilderPiece, gc_allocator_ignore_off_page<StringBuilderPiece>, 200> m_pieces;
};
}

#endif
