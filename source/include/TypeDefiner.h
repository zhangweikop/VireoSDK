/**
 
Copyright (c) 2014 National Instruments Corp.
 
This software is subject to the terms described in the LICENSE.TXT file
 
SDG
*/

/*! \file
    \brief Tools for registering Vireo types defined in C++ modules
 */

#ifndef TypeDefiner_h
#define TypeDefiner_h

#include "TypeAndDataManager.h"

//------------------------------------------------------------
namespace Vireo {

typedef void (*TypeDefinerCallback)(TypeManager& typeManager);

//! Facilitate the registration of Vireo types that are defined in C++ code.
class TypeDefiner
{
private:
    TypeDefiner*            _pNext;
    TypeDefinerCallback     _pCallback;
    const char*             _pNameSpace;
public :
    //! Add core primitive types to the specified TypeManager
    static void DefineStandardTypes(TypeManager& tm);
    //! Add registered types to the specified TypeManager
    static void DefineTypes(TypeManager& tm);

    
    //@{
    /** Methods used by C++ modules to register Vireo type definitions. */
    TypeDefiner(TypeDefinerCallback pCallback, const char* pNameSapce, Int32 version);
    static TypeRef Define(TypeManager& tm, const char* name, const char* typeCStr);
    static TypeRef Define(TypeManager& tm, SubString* name, SubString* wrappedTypeString);
    static TypeRef ParseAndBuidType(TypeManager& tm, SubString* typeString);
    static void DefineCustomPointerTypeWithValue(TypeManager& tm, const char* name, void* instruction, const char* typeString,PointerTypeEnum pointerType);
    static void DefineCustomValue(TypeManager& tm, const char* name, Int32 value, const char* typeString);
    static void DefineCustomDataProcs(TypeManager& tm, const char* name, IDataProcs* pDataProcs, const char* typeString);
    //@}
    
};

}

#define TOKENPASTE(x, y, z)    x ## y ## z
#define TOKENPASTE2(x, y, z)   TOKENPASTE(x, y, z)


#ifdef VIREO_MICRO
    // The micro verison function does not used dynamic registration.
    
    #define DEFINE_VIREO_BEGIN(_section_)
    #define DEFINE_VIREO_END()

    #define DEFINE_VIREO_TYPE(_name_, _type_) \

    #define DEFINE_VIREO_FUNCTION(_name_, _typeTypeString_)

    #define DEFINE_VIREO_FUNCTION_NAME(_symbol_, _name_, _typeTypeString_)

    #define DEFINE_VIREO_VALUE(_name_, value, _typeTypeString_)

    #define DEFINE_VIREO_GENERIC(_name_, _typeTypeString_, _genericEmitProc_)

    #define DEFINE_VIREO_CUSTOM_DP(_name_, _type_, _allocClass_)

#else

#define DEFINE_VIREO_BEGIN(_section_) \
static void TOKENPASTE2(DefineTypes, _section_, __LINE__) (TypeManager& tm); \
static TypeDefiner TOKENPASTE2(TheTypeDefiner, _section_, __LINE__) (TOKENPASTE2(DefineTypes, _section_, __LINE__), #_section_, kVireoABIVersion); \
static void TOKENPASTE2(DefineTypes, _section_, __LINE__) (TypeManager& tm) {

#define DEFINE_VIREO_END()   }


    #define DEFINE_VIREO_TYPE(_name_, _type_) \
    (TypeDefiner::Define(tm, #_name_, _type_));

    #define DEFINE_VIREO_FUNCTION(_name_, _typeTypeString_) \
    (TypeDefiner::DefineCustomPointerTypeWithValue(tm, #_name_, (void*)_name_, _typeTypeString_, kPTInstructionFunction));

    #define DEFINE_VIREO_FUNCTION_NAME(_symbol_, _name_, _typeTypeString_) \
    (TypeDefiner::DefineCustomPointerTypeWithValue(tm, #_name_, (void*)_symbol_, _typeTypeString_, kPTInstructionFunction));

    #define DEFINE_VIREO_VALUE(_name_, value, _typeTypeString_) \
    (TypeDefiner::DefineCustomValue(tm, #_name_, value, _typeTypeString_));

    #define DEFINE_VIREO_GENERIC(_name_, _typeTypeString_, _genericEmitProc_) \
    (TypeDefiner::DefineCustomPointerTypeWithValue(tm, #_name_, (void*)_genericEmitProc_, _typeTypeString_, kPTGenericFunctionCodeGen));

    #define DEFINE_VIREO_CUSTOM_DP(_name_, _type_, _allocClass_) \
    (TypeDefiner::DefineCustomDataProcs(tm, #_name_, _allocClass_, _type_));

#endif

#endif // TypeDefiner_h

