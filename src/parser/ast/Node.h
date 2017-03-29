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

#ifndef Node_h
#define Node_h

#include "runtime/AtomicString.h"
#include "runtime/Value.h"

namespace Escargot {

class ByteCodeBlock;
class ByteCodeGenerateContext;

enum ASTNodeType {
    ASTNodeTypeError,
    Program,
    Function,
    FunctionExpression,
    Property,
    Statement,
    Empty,
    EmptyStatement,
    BlockStatement,
    BreakStatement,
    BreakLabelStatement,
    ContinueStatement,
    ContinueLabelStatement,
    ReturnStatement,
    IfStatement,
    ForStatement,
    ForInStatement,
    WhileStatement,
    DoWhileStatement,
    SpreadElement,
    RestElement,
    SwitchStatement,
    SwitchCase,
    WithStatement,
    Declaration,
    VariableDeclaration,
    FunctionDeclaration,
    Pattern,
    Expression,
    ThisExpression,
    ExpressionStatement,
    ArrayExpression,
    UnaryExpressionBitwiseNot,
    UnaryExpressionDelete,
    UnaryExpressionLogicalNot,
    UnaryExpressionMinus,
    UnaryExpressionPlus,
    UnaryExpressionTypeOf,
    UnaryExpressionVoid,
    AssignmentExpression,
    AssignmentExpressionBitwiseAnd,
    AssignmentExpressionBitwiseOr,
    AssignmentExpressionBitwiseXor,
    AssignmentExpressionDivision,
    AssignmentExpressionLeftShift,
    AssignmentExpressionMinus,
    AssignmentExpressionMod,
    AssignmentExpressionMultiply,
    AssignmentExpressionPlus,
    AssignmentExpressionSignedRightShift,
    AssignmentExpressionUnsignedRightShift,
    AssignmentExpressionSimple,
    BinaryExpression,
    BinaryExpressionBitwiseAnd,
    BinaryExpressionBitwiseOr,
    BinaryExpressionBitwiseXor,
    BinaryExpressionDivison,
    BinaryExpressionEqual,
    BinaryExpressionGreaterThan,
    BinaryExpressionGreaterThanOrEqual,
    BinaryExpressionIn,
    BinaryExpressionInstanceOf,
    BinaryExpressionLeftShift,
    BinaryExpressionLessThan,
    BinaryExpressionLessThanOrEqual,
    BinaryExpressionLogicalAnd,
    BinaryExpressionLogicalOr,
    BinaryExpressionMinus,
    BinaryExpressionMod,
    BinaryExpressionMultiply,
    BinaryExpressionNotEqual,
    BinaryExpressionNotStrictEqual,
    BinaryExpressionPlus,
    BinaryExpressionSignedRightShift,
    BinaryExpressionStrictEqual,
    BinaryExpressionUnsignedRightShift,
    LogicalExpression,
    UpdateExpressionDecrementPostfix,
    UpdateExpressionDecrementPrefix,
    UpdateExpressionIncrementPostfix,
    UpdateExpressionIncrementPrefix,
    ObjectExpression,
    SequenceExpression,
    NewExpression,
    MemberExpression,
    YieldExpression,
    ConditionalExpression,
    CallExpression,
    VariableDeclarator,
    Identifier,
    LabeledStatement,
    Literal,
    Directive,
    RegExpLiteral,
    NativeFunction,
    TryStatement,
    CatchClause,
    ThrowStatement,
};

struct NodeLOC {
    size_t index;
    NodeLOC(size_t index)
    {
        this->index = index;
    }
};

struct ExtendedNodeLOC {
    size_t line;
    size_t column;
    size_t index;

    ExtendedNodeLOC(size_t line, size_t column, size_t index)
    {
        this->line = line;
        this->column = column;
        this->index = index;
    }
};

class LiteralNode;
class IdentifierNode;
class MemberExpressionNode;

class Node : public gc {
    friend class ScriptParser;

protected:
    Node()
        : m_loc(SIZE_MAX)
    {
    }

public:
    virtual ~Node()
    {
    }

    virtual ASTNodeType type() = 0;

    bool isIdentifier()
    {
        return type() == ASTNodeType::Identifier;
    }

    IdentifierNode *asIdentifier()
    {
        ASSERT(isIdentifier());
        return (IdentifierNode *)this;
    }

    MemberExpressionNode *asMemberExpression()
    {
        ASSERT(isMemberExpression());
        return (MemberExpressionNode *)this;
    }

    LiteralNode *asLiteral()
    {
        ASSERT(isLiteral());
        return (LiteralNode *)this;
    }

    bool isLiteral()
    {
        return type() == ASTNodeType::Literal;
    }

    bool isMemberExpression()
    {
        return type() == ASTNodeType::MemberExpression;
    }

    virtual bool isExpressionNode()
    {
        return false;
    }

    virtual bool isStatementNode()
    {
        return false;
    }

    NodeLOC loc()
    {
        return m_loc;
    }

    virtual void generateStatementByteCode(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context)
    {
        RELEASE_ASSERT_NOT_REACHED();
    }

    virtual void generateExpressionByteCode(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context, ByteCodeRegisterIndex dstRegister)
    {
        RELEASE_ASSERT_NOT_REACHED();
    }

    virtual void generateStoreByteCode(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context, ByteCodeRegisterIndex srcRegister, bool needToReferenceSelf);

    virtual void generateResolveAddressByteCode(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context)
    {
    }

    virtual void generateReferenceResolvedAddressByteCode(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context);
    virtual void generateResultNotRequiredExpressionByteCode(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context);

    virtual ByteCodeRegisterIndex getRegister(ByteCodeBlock *codeBlock, ByteCodeGenerateContext *context);

    NodeLOC m_loc;
};

class Pattern {
public:
};

struct ASTScoptContextNameInfo {
    bool m_isExplicitlyDeclaredOrParameterName;
    AtomicString m_name;
};

typedef Vector<ASTScoptContextNameInfo, GCUtil::gc_malloc_atomic_ignore_off_page_allocator<ASTScoptContextNameInfo>> ASTScoptContextNameInfoVector;

struct ASTScopeContext : public gc {
    bool m_isStrict : 1;
    bool m_hasEval : 1;
    bool m_hasWith : 1;
    bool m_hasCatch : 1;
    bool m_hasYield : 1;
    bool m_inCatch : 1;
    bool m_inWith : 1;
    bool m_hasManyNumeralLiteral : 1;
    ASTNodeType m_nodeType;
    ASTScopeContext *m_parentContext;
    Node *m_associateNode;
    ASTScoptContextNameInfoVector m_names;
    AtomicStringVector m_usingNames;
    AtomicStringVector m_parameters;
    AtomicString m_functionName;
    Vector<ASTScopeContext *, GCUtil::gc_malloc_ignore_off_page_allocator<ASTScopeContext *>> m_childScopes;
    Vector<Value, GCUtil::gc_malloc_atomic_ignore_off_page_allocator<Value>> *m_numeralLiteralData;
    ExtendedNodeLOC m_locStart;
    ExtendedNodeLOC m_locEnd;

    void *operator new(size_t size);
    void *operator new[](size_t size) = delete;

    void insertName(AtomicString name, bool isExplicitlyDeclaredOrParameterName)
    {
        for (size_t i = 0; i < m_names.size(); i++) {
            if (m_names[i].m_name == name) {
                if (isExplicitlyDeclaredOrParameterName) {
                    m_names[i].m_isExplicitlyDeclaredOrParameterName = isExplicitlyDeclaredOrParameterName;
                }
                return;
            }
        }

        ASTScoptContextNameInfo info;
        info.m_name = name;
        info.m_isExplicitlyDeclaredOrParameterName = isExplicitlyDeclaredOrParameterName;
        m_names.push_back(info);
    }

    void insertUsingName(AtomicString name)
    {
        if (VectorUtil::findInVector(m_usingNames, name) == VectorUtil::invalidIndex) {
            m_usingNames.push_back(name);
        }
    }

    void insertNumeralLiteral(Value v)
    {
        ASSERT(!v.isPointerValue());
        if (!m_numeralLiteralData)
            m_numeralLiteralData = new (GC) Vector<Value, GCUtil::gc_malloc_atomic_ignore_off_page_allocator<Value>>();
        if (VectorUtil::findInVector(*m_numeralLiteralData, v) == VectorUtil::invalidIndex) {
            if (m_numeralLiteralData->size() < KEEP_NUMERAL_LITERDATA_IN_REGISTERFILE_LIMIT)
                m_numeralLiteralData->push_back(v);
            else {
                m_numeralLiteralData->clear();
                m_hasManyNumeralLiteral = true;
            }
        }
    }

    ASTScopeContext(bool isStrict, ASTScopeContext *parentContext)
        : m_locStart(SIZE_MAX, SIZE_MAX, SIZE_MAX)
        , m_locEnd(SIZE_MAX, SIZE_MAX, SIZE_MAX)
    {
        m_isStrict = isStrict;
        m_hasYield = m_hasCatch = m_hasWith = m_hasEval = false;
        m_hasManyNumeralLiteral = m_inCatch = m_inWith = false;
        m_parentContext = parentContext;
        m_associateNode = nullptr;
        m_numeralLiteralData = nullptr;
    }
};

typedef Vector<Node *, gc_allocator_ignore_off_page<Node *>> NodeVector;
typedef Vector<Node *, gc_allocator_ignore_off_page<Node *>> ArgumentVector;
typedef Vector<Node *, gc_allocator_ignore_off_page<Node *>> ExpressionNodeVector;
typedef Vector<Node *, gc_allocator_ignore_off_page<Node *>> StatementNodeVector;
typedef Vector<Node *, gc_allocator_ignore_off_page<Node *>> PatternNodeVector;
class PropertyNode;
typedef Vector<PropertyNode *, gc_allocator_ignore_off_page<PropertyNode *>> PropertiesNodeVector;
class VariableDeclaratorNode;
typedef Vector<VariableDeclaratorNode *, gc_allocator_ignore_off_page<VariableDeclaratorNode *>> VariableDeclaratorVector;
}

#endif
