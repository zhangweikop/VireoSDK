/**
 
Copyright (c) 2014 National Instruments Corp.
 
This software is subject to the terms described in the LICENSE.TXT file
 
SDG
*/

/*! \file
    \brief A Vireo codec for the LabVIEW binary dlattened data format.
 */

namespace Vireo
{

NIError FlattenData(TypeRef type, void *pData, StringRef pString, Boolean prependArrayLength);

IntIndex UnflattenData(StringRef pString, Boolean prependArrayLength, IntIndex stringIndex, void *pDefaultData, TypeRef type, void *pData);

} // namespace Vireo
