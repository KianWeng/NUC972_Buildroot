/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the qt3to4 porting application of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "codemodelwalker.h"

QT_BEGIN_NAMESPACE
using namespace CodeModel;

void CodeModelWalker::parseScope(CodeModel::Scope *scope)
{
    if(!scope)
        return;

    if(scope->toClassScope())
        parseClassScope(scope->toClassScope());
    if(scope->toNamespaceScope())
        parseNamespaceScope(scope->toNamespaceScope());
    if(scope->toBlockScope())
        parseBlockScope(scope->toBlockScope());


    {
        MemberCollection collection = scope->members();
        MemberCollection::ConstIterator it = collection.constBegin();
        while(it != collection.constEnd())
            parseMember(*it++);
    }
    {
        ScopeCollection collection = scope->scopes();
        ScopeCollection::ConstIterator it = collection.constBegin();
        while(it != collection.constEnd())
            parseScope(*it++);
    }
    {
        NameUseCollection collection = scope->nameUses();
        NameUseCollection::ConstIterator it = collection.constBegin();
        while(it != collection.constEnd())
            parseNameUse(*it++);
    }
}

void CodeModelWalker::parseType(CodeModel::Type *type)
{
    if(!type)
        return;
    if (type->toEnumType())
        parseEnumType(type->toEnumType());
    else if (type->toClassType())
        parseClassType(type->toClassType());
    else if (type->toBuiltinType())
        parseBuiltinType(type->toBuiltinType());
    else if (type->toPointerType())
        parsePointerType(type->toPointerType());
    else if (type->toReferenceType())
        parseReferenceType(type->toReferenceType());
    else if (type->toGenericType())
        parseGenericType(type->toGenericType());
    else if (type->toAliasType())
        parseAliasType(type->toAliasType());
    else if (type->toUnknownType())
        parseUnknownType(type->toUnknownType());
}

void CodeModelWalker::parseMember(CodeModel::Member *member)
{
    if(!member)
        return;

    if (member->toFunctionMember())
        parseFunctionMember(member->toFunctionMember());
    else if (member->toVariableMember())
        parseVariableMember(member->toVariableMember());
    else if (member->toUsingDeclarationMember())
        parseUsingDeclarationMember(member->toUsingDeclarationMember());
    else if (member->toTypeMember())
        parseTypeMember(member->toTypeMember());
}

void CodeModelWalker::parseFunctionMember(CodeModel::FunctionMember *member)
{
    if(!member)
        return;
    if(member->functionBodyScope())
        parseScope(member->functionBodyScope());
}

QT_END_NAMESPACE
