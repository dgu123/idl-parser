// -*- mode: c++; c-basic-style: "bsd"; c-basic-offset: 4; -*-
/*
 * idlmm/EnumDef.hpp
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

#ifndef IDLMM_ENUMDEF_HPP
#define IDLMM_ENUMDEF_HPP

#include <idlmm_forward.hpp>

#include <idlmm/TypedefDef.hpp>

/*PROTECTED REGION ID(EnumDef_pre) START*/
// Please, enable the protected region if you add manually written code.
// To do this, add the keyword ENABLED before START.
/*PROTECTED REGION END*/

namespace idlmm
{

    class EnumDef: public virtual ::idlmm::TypedefDef
    {
    public:
        EnumDef();

        virtual ~EnumDef();

        // Operations
        // Attributes

        // References
        typedef boost::ptr_vector< ::idlmm::EnumMember > Members_t;
        Members_t& getMembers();

        /*PROTECTED REGION ID(EnumDef) START*/
        // Please, enable the protected region if you add manually written code.
        // To do this, add the keyword ENABLED before START.
        /*PROTECTED REGION END*/

        // EObjectImpl
        /*PROTECTED REGION ID(EnumDefImpl) ENABLED START*/

        virtual void accept(IdlmmVisitor& visitor);

        // Please, enable the protected region if you add manually written code.
        // To do this, add the keyword ENABLED before START.
        /*PROTECTED REGION END*/

    protected:
        // Attributes
        // References

        boost::ptr_vector<
                ::idlmm::EnumMember > m_members;

    };

} // idlmm

#endif // IDLMM_ENUMDEF_HPP
