// -*- mode: c++; c-basic-style: "bsd"; c-basic-offset: 4; -*-
/*
 * idlmm/EnumMember.cpp
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

#include "EnumMember.hpp"
#include "IdlmmVisitor.hpp"
#include <idlmm/Constant.hpp>
#include <idlmm/Container.hpp>
#include <idlmm/EnumDef.hpp>
using namespace ::idlmm;

// Default constructor
EnumMember::EnumMember() :
    m_enum(0)
{

    /*PROTECTED REGION ID(EnumMemberImpl__EnumMemberImpl) START*/
    // Please, enable the protected region if you add manually written code.
    // To do this, add the keyword ENABLED before START.
    /*PROTECTED REGION END*/

}

EnumMember::~EnumMember()
{
}

/*PROTECTED REGION ID(EnumMember.cpp) ENABLED START*/

void EnumMember::accept(IdlmmVisitor& visitor)
{
    visitor.visit(this);
}

// Please, enable the protected region if you add manually written code.
// To do this, add the keyword ENABLED before START.
/*PROTECTED REGION END*/

// Attributes

// References
::idlmm::EnumDef_ptr EnumMember::getEnum()
{
    return m_enum;
}

void EnumMember::setEnum(::idlmm::EnumDef_ptr _enum)
{
    m_enum = _enum;
}

