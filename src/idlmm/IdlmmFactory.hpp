// -*- mode: c++; c-basic-style: "bsd"; c-basic-offset: 4; -*-
/*
 * idlmm/IdlmmFactory.hpp
 * Copyright (C) Cátedra SAES-UMU 2010 <andres.senac@um.es>
 *
 * EMF4CPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EMF4CPP is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _IDLMMFACTORY_HPP
#define _IDLMMFACTORY_HPP
#include <idlmm.hpp>

namespace idlmm
{

    class IdlmmFactory
    {
    public:

        static IdlmmFactory_ptr _instance();

        virtual ModelElement_ptr createModelElement();
        virtual Container_ptr createContainer();
        virtual Contained_ptr createContained();
        virtual InterfaceDef_ptr createInterfaceDef();
        virtual ModuleDef_ptr createModuleDef();
        virtual IDLType_ptr createIDLType();
        virtual OperationDef_ptr createOperationDef();
        virtual AttributeDef_ptr createAttributeDef();
        virtual ConstantDef_ptr createConstantDef();
        virtual Typed_ptr createTyped();
        virtual ParameterDef_ptr createParameterDef();
        virtual PrimitiveDef_ptr createPrimitiveDef();
        virtual ExceptionDef_ptr createExceptionDef();
        virtual Field_ptr createField();
        virtual FixedDef_ptr createFixedDef();
        virtual WstringDef_ptr createWstringDef();
        virtual StringDef_ptr createStringDef();
        virtual AliasDef_ptr createAliasDef();
        virtual ArrayDef_ptr createArrayDef();
        virtual SequenceDef_ptr createSequenceDef();
        virtual UnionField_ptr createUnionField();
        virtual TypedefDef_ptr createTypedefDef();
        virtual UnionDef_ptr createUnionDef();
        virtual EnumDef_ptr createEnumDef();
        virtual StructDef_ptr createStructDef();
        virtual TranslationUnit_ptr createTranslationUnit();
        virtual Include_ptr createInclude();
        virtual Expression_ptr createExpression();
        virtual BinaryExpression_ptr createBinaryExpression();
        virtual UnaryExpression_ptr createUnaryExpression();
        virtual LiteralExpression_ptr createLiteralExpression();
        virtual ConstantDefRef_ptr createConstantDefRef();
        virtual ValueExpression_ptr createValueExpression();
        virtual ForwardDef_ptr createForwardDef();
        virtual EnumMember_ptr createEnumMember();
        virtual Constant_ptr createConstant();
        virtual NamedElement_ptr createNamedElement();

    protected:

        IdlmmFactory();

    };

} // idlmm
#endif // _IDLMMFACTORY_HPP
