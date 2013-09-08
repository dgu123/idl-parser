#ifndef IDL_PARSER_IPP
#define IDL_PARSER_IPP

#include <iostream>
#include <fstream>
#include <bitset>
#include <map>
#include <vector>
#include <deque>
#include <cassert>
#include "idl_grammar.hpp"
#include <ecore.hpp>
#include <idlmm.hpp>

namespace idl 
{
namespace parser 
{

const char * NSS = "::"; // namespace separator

template < typename Set >
idlmm::ParameterMode get_direction(const Set& s)
{
    using namespace grammar;
    using namespace idlmm;
    using namespace ecore;

    IdlmmPackage_ptr p = IdlmmPackage::_instance(); 
    EEnum_ptr e = p->getParameterMode();
    const char * str = NULL;

    if (s.test(FLAG_OUT))
        str = "PARAM_OUT";
    else if (s.test(FLAG_INOUT))
        str = "PARAM_INOUT";
    else
        str = "PARAM_IN";

    EEnumLiteral_ptr l = e->getEEnumLiteral(str);
    assert(l);
    return l->getValue();
}

const char * to_string(idl::grammar::primitive_types t)
{
    using namespace grammar;
    switch (t)
    {
        case PT_UNSIGNED_SHORT:
            return "PK_USHORT";
        case PT_UNSIGNED_LONG_LONG:
            return "PK_ULONGLONG";
        case PT_UNSIGNED_LONG:
            return "PK_ULONG";
        case PT_LONG_DOUBLE:
            return "PK_LONGDOUBLE";
        case PT_LONG_LONG:
            return "PK_LONGLONG";
        case PT_LONG:
            return "PK_LONG";
        case PT_OCTET:
            return "PK_OCTET";
        case PT_CHAR:
            return "PK_CHAR";
        case PT_SHORT:
            return "PK_SHORT";
        case PT_FLOAT:
            return "PK_FLOAT";
        case PT_DOUBLE:
            return "PK_DOUBLE";
        case PT_STRING:
            return "PK_STRING";
        case PT_WSTRING:
            return "PK_WSTRING";
        case PT_VOID:
            return "PK_VOID";
        case PT_NULL:
        default:
            return "PK_NULL";
    }

    return NULL;
}

idlmm::PrimitiveKind get_primitive_kind(idl::grammar::primitive_types t)
{
    using namespace ecore;
    using namespace idlmm;

    IdlmmPackage_ptr p = IdlmmPackage::_instance(); 
    EEnum_ptr e = p->getPrimitiveKind();
    EEnumLiteral_ptr l = e->getEEnumLiteral(to_string(t));
    assert(l);
    return l->getValue();
}

template < typename T >
struct lookupInto
{
    template< typename Scope, typename C >
    static inline void apply(const Scope&, const std::string&, const C *)
    {
    }
};

template < >
struct lookupInto< idlmm::Constant >
{
    template< typename Scope, typename C >
    static inline void apply(Scope& s, const std::string& prefix, C * c)
    {
        using namespace idlmm;
        EnumDef_ptr e = c->template as< EnumDef >();

        if (e)
        {
            for (std::size_t i = 0; i < e->getMembers().size(); i++) 
            {
                EnumMember_ptr m = e->getMembers()[i];
                s[prefix + m->getIdentifier()] = m;
            }
        }
    }
};

struct SemanticState
{
    typedef grammar::semantic_context_type context_type_t;
    typedef std::vector< ecore::EObject * > objects_t;

    struct Context
    {
        context_type_t context_type;
        std::string identifier;
        std::string data;
        std::bitset< sizeof(void *) * 8 > flags; 
        objects_t& objects;
        std::size_t prev_size;
        grammar::primitive_types primitive_type;

        Context(objects_t& objects_,
                context_type_t context_type_) : 
            context_type(context_type_),
            objects(objects_),
            prev_size(objects_.size()),
            primitive_type(grammar::PT_NULL)
        {}

        // restores previous size, discards current objects
        void clear()
        {
            objects.resize(prev_size);
        }

        ~Context()
        {
            // No memory leaks due to rollback
            for (std::size_t i = prev_size; i < objects.size(); i++) 
            {
                delete objects[i];
            }

            clear();
        }
    };

    ecore::EObject * result;

    typedef std::deque< Context > contexts_t; 
    contexts_t contexts;

    objects_t objects;

    typedef std::vector< std::string > literals_t;
    literals_t literals;

    SemanticState() : result (NULL) {}

    void push_identifier(const std::string& id)
    {
        contexts.back().identifier = id;
    }

    void push_literal(const std::string& l)
    {
        literals.push_back(l);
    }

    void new_context(context_type_t type)
    {
        contexts.push_back(Context(objects, type));
    }

    void set_flag(grammar::flags flag)
    {
        contexts.back().flags.set(flag);
    }

    void set_primitive_type(grammar::primitive_types type)
    {
        contexts.back().primitive_type = type;
    }

    template < class Contained, class Obj, typename Fn >
    void populate(Context& c, Obj * o, Fn f)
    {
        for (std::size_t i = c.prev_size; i < c.objects.size(); i++) 
        {
            Contained * t = c.objects[i]->as< Contained >();
            assert(t);
            if (t) (o->*f)().push_back(t);
        }

        // avoid double free
        c.clear();
    }

    // for those classes that can have more than one kind of children
    template < class Contained, class Obj, typename Fn >
    void populateIf(Context& c, Obj * o, Fn f)
    {
        for (std::size_t i = c.prev_size; i < c.objects.size(); i++) 
        {
            Contained * t = c.objects[i]->as< Contained >();
            if (t) (o->*f)().push_back(t);
        }
    }

    // attempt to implement a lookup function
    // TODO must be optimized
    // TODO merge equivalent namespaces
    template < typename Type >
    Type* lookup(const std::string& f)
    {
        typedef Type* Type_ptr;
        typedef std::map< std::string, Type_ptr > scope_t;

        scope_t scope;
        std::size_t size = objects.size();

        for (contexts_t::reverse_iterator it = contexts.rbegin(); 
                it != contexts.rend(); it++) 
        {
            // previous scope transformation
            if (!it->identifier.empty() && !scope.empty())
            {
                scope_t old (scope);
                scope.clear();

                for (typename scope_t::iterator jt = old.begin(); 
                        jt != old.end(); ++jt) 
                {
                    scope[it->identifier + NSS + jt->first] = jt->second;
                }
            }

            typedef std::vector< std::pair< std::string, idlmm::Container_ptr > > 
                containers_t;
            containers_t containers;

            // current objects
            for (std::size_t i = it->prev_size; i < size; i++) 
            {
                Type_ptr t = 
                    objects[i]->as< Type >(); 

                if (t)
                    scope[t->getIdentifier()] = t;

                lookupInto< Type >::apply(scope, "", objects[i]);

                idlmm::Container_ptr c =
                    objects[i]->as< idlmm::Container >();

                if (c)
                    containers.push_back(std::make_pair(c->getIdentifier(), c));
            }
            // next iteration
            size = it->prev_size;

            // Recurse into the containers
            for (std::size_t i = 0; i < containers.size(); i++) 
            {
                idlmm::Container_ptr c = containers[i].second;
                const std::string& first = containers[i].first;
                const std::string prefix = 
                    (first.empty())? first: first + NSS; 

                for (std::size_t j = 0; j < c->getContains().size(); j++) 
                {
                    Type_ptr t = 
                        c->getContains()[j]->as< Type  >(); 

                    if (t)
                        scope[prefix + t->getIdentifier()] = t;

                    lookupInto< Type >::apply(scope, prefix, objects[i]);

                    idlmm::Container_ptr r =
                        c->getContains()[j]->as< idlmm::Container >();

                    if (r)
                        containers.push_back(std::make_pair(
                                    prefix + r->getIdentifier(), r));
                }
            }

            typename scope_t::iterator i = scope.find(f);
            if (i != scope.end())
                return i->second;
        }

        std::cerr << "Error: undefined reference to " << f << std::endl;
        std::cerr << "Current scope:" << std::endl;
        for (typename scope_t::const_iterator it = scope.begin(); 
                it != scope.end(); ++it) 
        {
            std::cerr << "  " << it->first << std::endl;
        }

        return NULL;
    }

    template < typename Typed >
    bool try_to_set_type(Typed * typed, 
            void (Typed::*setContainedType)(idlmm::IDLType_ptr) = 
                &Typed::setContainedType,
            void (Typed::*setSharedType)(idlmm::TypedefDef_ptr) = 
                &Typed::setSharedType)
    {
        using namespace idlmm;

        const Context& c = contexts.back();
        bool res = false;

        if (c.primitive_type != grammar::PT_NULL)
        {
            IdlmmFactory_ptr f = IdlmmFactory::_instance(); 
            PrimitiveDef * p = f->createPrimitiveDef();
            p->setKind(get_primitive_kind(c.primitive_type));
            (typed->*setContainedType)(p);
            res = true;
        }
        else if (!c.data.empty())
        {
            idlmm::TypedefDef_ptr t = lookup< idlmm::TypedefDef >(c.data);
            if (t)
            {
                (typed->*setSharedType)(t);
                res = true;
            }
        }

        return res;
    }

    bool try_to_set_type_to_alias(idlmm::AliasDef_ptr typed)
    {
        Context& c = contexts.back();
        if (c.prev_size != objects.size())
        {
            assert(objects.size() == c.prev_size + 1);

            idlmm::IDLType_ptr t = 
                objects[c.prev_size]->as< idlmm::IDLType >();
            assert(t);
            typed->setContainedType(t);

            c.clear();

            return true;
        }
        return try_to_set_type< idlmm::Typed >(typed);
    }
    
    template < typename S, typename match_pair >
    void set_context_data(S& state, match_pair const& mp)
    {
    }

    template < typename S, typename match_pair >
    void set_data(S& state, match_pair const& mp)
    {
        contexts.back().data = state.to_string(mp.first, mp.second);
    }

    void commit()
    {
        using namespace idlmm;
        using namespace grammar;

        IdlmmFactory_ptr f = IdlmmFactory::_instance(); 

        Context& c = contexts.back();
        const std::size_t diff = objects.size() - c.prev_size;

        ecore::EObject_ptr obj = NULL;

        switch (c.context_type)
        {
        case CONTEXT_TRANSLATION_UNIT:
            {
                TranslationUnit_ptr o = f->createTranslationUnit();
                o->setIdentifier(c.identifier);

                populate< Contained >(c, o, 
                        &TranslationUnit::getContains);

                obj = o;
            }
            break;
        case CONTEXT_MODULE:
            {
                ModuleDef_ptr o = f->createModuleDef();
                o->setIdentifier(c.identifier);

                populate< Contained >(c, o, &ModuleDef::getContains);

                obj = o;
            }
            break;
        case CONTEXT_STRUCT:
            {
                StructDef_ptr o = f->createStructDef();
                o->setIdentifier(c.identifier);

                populate< Field >(c, o, &StructDef::getMembers);

                obj = o;
            }
            break;
        case CONTEXT_EXCEPTION:
            {
                ExceptionDef_ptr o = f->createExceptionDef();
                o->setIdentifier(c.identifier);

                populate< Field >(c, o, &ExceptionDef::getMembers);

                obj = o;
            }
            break;
        case CONTEXT_UNION:
            {
                UnionDef_ptr o = f->createUnionDef();
                o->setIdentifier(c.identifier);

                populate< UnionField >(c, o, &UnionDef::getUnionMembers);

                // discriminator
                try_to_set_type< UnionDef >(o, 
                        &UnionDef::setContainedDiscrim,
                        &UnionDef::setSharedDiscrim);

                obj = o;
            }
            break;
        case CONTEXT_INTERFACE:
            {
                InterfaceDef_ptr o = f->createInterfaceDef();
                o->setIdentifier(c.identifier);
                o->setIsAbstract(c.flags.test(FLAG_ABSTRACT));

                populate< Contained >(c, o, &InterfaceDef::getContains);

                obj = o;
            }
            break;
        case CONTEXT_INTERFACE_FWD:
            {
                ForwardDef_ptr o = f->createForwardDef();
                o->setIdentifier(c.identifier);
                // Error in the model 
                // o->setIsAbstract(c.flags.test(FLAG_ABSTRACT));

                obj = o;
            }
            break;
        case CONTEXT_OPERATION:
            {
                OperationDef_ptr o = f->createOperationDef();
                o->setIdentifier(c.identifier);
                o->setIsOneway(c.flags.test(FLAG_ONEWAY));

                populate< ParameterDef >(c, o, 
                        &OperationDef::getParameters);

                try_to_set_type< Typed >(o);

                obj = o;
            }
            break;
        case CONTEXT_PARAMETER:
            {
                ParameterDef_ptr o = f->createParameterDef();
                o->setIdentifier(c.identifier);
                o->setDirection(get_direction(c.flags));

                try_to_set_type< Typed >(o);

                obj = o;
            }
            break;
        case CONTEXT_ATTRIBUTE:
            {
                AttributeDef_ptr o = f->createAttributeDef();
                o->setIdentifier(c.identifier);
                o->setIsReadonly(c.flags.test(FLAG_READONLY));

                try_to_set_type< Typed >(o);

                obj = o;
            }
            break;
        case CONTEXT_STRUCT_FIELD:
            {
                Field_ptr o = f->createField();
                o->setIdentifier(c.identifier);

                try_to_set_type< Typed >(o);

                obj = o;
            }
            break;
        case CONTEXT_UNION_FIELD:
            {
                UnionField_ptr o = f->createUnionField();
                o->setIdentifier(c.identifier);

                try_to_set_type< Typed >(o);
                
                populate< Expression >(c, o, &UnionField::getLabel);

                obj = o;
            }
            break;
        case CONTEXT_ALIAS:
            {
                AliasDef_ptr o = f->createAliasDef();
                o->setIdentifier(c.identifier);

                try_to_set_type_to_alias(o);

                obj = o;
            }
            break;
        case CONTEXT_ARRAY:
            {
                AliasDef_ptr o = f->createAliasDef();
                o->setIdentifier(c.identifier);

                ArrayDef_ptr a = f->createArrayDef();
                try_to_set_type< Typed >(a);
                o->setContainedType(a);

                // bounds
                assert(diff > 0);
                for (std::size_t i = c.prev_size; i < objects.size(); i++) 
                {
                    a->getBounds().push_back(objects[i]->as< Expression >());
                }

                c.clear();
                obj = o;
            }
            break;
        case CONTEXT_ENUM:
            {
                EnumDef_ptr o = f->createEnumDef();
                o->setIdentifier(c.identifier);

                for (literals_t::const_iterator it = literals.begin(); 
                        it != literals.end(); ++it) 
                {
                    EnumMember_ptr m = f->createEnumMember();
                    m->setIdentifier(*it);
                    o->getMembers().push_back(m);
                }
                literals.clear();

                obj = o;
            }
            break;
        case CONTEXT_SEQUENCE:
            {
                SequenceDef_ptr o = f->createSequenceDef();
                try_to_set_type< Typed >(o);

                // bound
                if (diff)
                {
                    assert(diff == 1);
                    o->setBound(objects[c.prev_size]->as< Expression >());
                    c.clear();
                }

                obj = o;
            }
            break;
         case CONTEXT_STRING:
            {
                StringDef_ptr o = f->createStringDef();

                if (diff)
                {
                    assert(diff == 1);
                    o->setBound(objects[c.prev_size]->as< Expression >());
                    c.clear();
                }

                obj = o;
            }
            break;
         case CONTEXT_WSTRING:
            {
                WstringDef_ptr o = f->createWstringDef();

                if (diff)
                {
                    assert(diff == 1);
                    o->setBound(objects[c.prev_size]->as< Expression >());
                    c.clear();
                }

                obj = o;
            }
            break;
        case CONTEXT_CONST:
            {
                ConstantDef_ptr o = f->createConstantDef();
                o->setIdentifier(c.identifier);

                try_to_set_type< Typed >(o);

                // constant value
                assert(diff == 1);
                o->setConstValue(objects[c.prev_size]->as< Expression >());

                c.clear();
                obj = o;
            }
            break;
        case CONTEXT_BINARY_EXPRESSION:
            {
                if (diff == 2)
                {
                    BinaryExpression_ptr o = f->createBinaryExpression();
                    o->setLeft(objects[c.prev_size]->as< Expression >());
                    o->setRight(objects[c.prev_size + 1]->as< Expression >());
                    o->setOperator(c.data);

                    c.clear();
                    obj = o;
                }
                else if (diff == 1)
                {
                    obj = objects[c.prev_size];
                    c.clear();
                }
                else
                    assert(false);
            }
            break;
        case CONTEXT_UNARY_EXPRESSION:
            {
                UnaryExpression_ptr o = f->createUnaryExpression();
                o->setExpression(objects[c.prev_size]->as< Expression >());
                o->setOperator(c.data);

                c.clear();
                obj = o;
            }
            break;
        case CONTEXT_VALUE_EXPRESSION:
            {
                ValueExpression_ptr o = f->createValueExpression();
                o->setValue(c.data);

                obj = o;
            }
            break;
        case CONTEXT_CONSTANT_REF:
            {
                ConstantDefRef_ptr o = f->createConstantDefRef();
                Constant_ptr r = lookup< Constant >(c.data);

                if (r)
                    o->setConstant(r);

                obj = o;
            }
            break;
        default:
            assert(false);
            break;
        }
        contexts.pop_back();

        if (!contexts.empty())
        {
            assert (obj); 
            if (obj) objects.push_back(obj);
        }
        else
        {
            assert (obj); 
            result = obj;
        }
    }

    void rollback()
    {
        contexts.pop_back();
        literals.clear();
    }
};

} // namespace parser
} // namespace idl

#endif /* IDL_PARSER_IPP */
