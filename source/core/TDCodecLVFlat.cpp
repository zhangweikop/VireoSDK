/**
 
Copyright (c) 2014 National Instruments Corp.
 
This software is subject to the terms described in the LICENSE.TXT file
 
SDG
*/

/*! \file 
    \brief Native Verio functions for LabVIEW's flat data format.
 */

#include "TypeDefiner.h"

// TODO: Code review
namespace Vireo
{

NIError FlattenData(TypeRef type, void *pData, StringRef pString, Boolean prependArrayLength)
{
    EncodingEnum encoding = type->BitEncoding();

    switch (encoding) {
        case kEncoding_Array:
        {
            TypedArrayCore *pArray = *(TypedArrayCore **) pData;
            if (pArray == null)
                return kNIError_kResourceNotFound;

            TypeRef elementType = type->GetSubElement(0);

            // Conditionally prepend an array with its length.
            // This is only optional for top-level data.  Arrays contained in
            // other data structures always include length information.
            if (prependArrayLength)
            {
                Int32 arrayLength = pArray->Length();

                pString->Append(sizeof(arrayLength), (Utf8Char*)&arrayLength);
            }

            if (elementType->IsFlat())
            {
                pString->Append(pArray->Length() * elementType->TopAQSize(), (Utf8Char*)pArray->BeginAt(0));
            }
            else
            {
                // Recursively call FlattenData on each element.
                // Arrays contained in other data structures always include
                // length information.
                size_t   elementLength = pArray->GetSlabLengths()[0];
                AQBlock1 *pEnd = pArray->BeginAt(0) + (pArray->GetDimensionLengths()[0] * elementLength);
                AQBlock1 *pElement = pArray->BeginAt(0);

                for (; pElement < pEnd; pElement += elementLength)
                    FlattenData(elementType, pElement, pString, true);

            }
            break;
        }
        case kEncoding_Cluster:
        {
            IntIndex count = type->SubElementCount();

            for (IntIndex j = 0; j < count; j++)
            {
                TypeRef elementType = type->GetSubElement(j);
                IntIndex offset = elementType->ElementOffset();
                AQBlock1* pElementData = (AQBlock1*)pData + offset;

                // Recursively call FlattenData on each element with
                // prependArrayLength set to true.
                FlattenData(elementType, pElementData, pString, true);
            }
            break;
        }
        default:
        {
            pString->Append(type->TopAQSize(), (Utf8Char*)pData);
            break;
        }
    }

    return kNIError_Success;
}

VIREO_FUNCTION_SIGNATURE4(FlattenToString, StaticType, void, Boolean, StringRef)
{
    TypeRef type = _ParamPointer(0);
    void *pData  = _ParamPointer(1);
    Boolean prependArrayLength = _ParamPointer(2) ? _Param(2) : true;
    String *pString = _Param(3);

    pString->Resize1D(0);
    FlattenData(type, pData, pString, prependArrayLength);

    return _NextInstruction();
}

IntIndex UnflattenData(StringRef pString, Boolean prependArrayLength, IntIndex stringIndex, void *pDefaultData, TypeRef type, void *pData)
{
    EncodingEnum encoding = type->BitEncoding();

    switch (encoding) {
        case kEncoding_Array:
        {
            TypedArrayCore *pArray = pData ? *(TypedArrayCore **) pData : null;
            TypeRef elementType = type->GetSubElement(0);
            Int32 arrayLength;

            // If length information precedes the array, read it.
            // Otherwise, infer it based on the size of the remaining string.
            if (prependArrayLength)
            {
                // If the string is long enough, read the array length
                if (stringIndex + (IntIndex)sizeof(arrayLength) <= pString->Length())
                {
                    arrayLength = *(Int32 *)pString->BeginAt(stringIndex);
                    stringIndex += sizeof(arrayLength);
                }
                else
                    return -1;
            }
            else
                arrayLength = (pString->Length() - stringIndex) / elementType->TopAQSize();

            // If the length is a sane value, resize the array.
            if (arrayLength <= pString->Length() - stringIndex)
            {
                if (pArray)
                    pArray->Resize1D(arrayLength);
            }
            else
                return -1;

            if (elementType->IsFlat())
            {
                Int32 copyLength = arrayLength * elementType->TopAQSize();

                // If the string is long enough, copy data.
                if (stringIndex + copyLength <= pString->Length())
                {
                    if (pArray)
                        memcpy(pArray->BeginAt(0), pString->BeginAt(stringIndex), copyLength);
                    stringIndex += copyLength;
                }
                else
                    return -1;
            }
            else if (pData)
            {
                // Recursively call UnflattenData for each element.
                // Arrays contained in other data structures always include
                // length information.
                size_t   elementLength = pArray->GetSlabLengths()[0];
                AQBlock1 *pEnd = pArray->BeginAt(0) + (pArray->GetDimensionLengths()[0] * elementLength);
                AQBlock1 *pElementData = pArray->BeginAt(0);

                for (; pElementData < pEnd; pElementData += elementLength)
                {
                    stringIndex = UnflattenData(pString, true, stringIndex, pDefaultData, elementType, pElementData);
                    if (stringIndex == -1)
                        return -1;
                }
            }
            else
            {
                // pData is null, so call UnflattenData recursing on elements of pDefaultData.
                // Arrays contained in other data structures always include
                // length information.
                TypedArrayCore *pDefaultArray = *(TypedArrayCore **) pDefaultData;
                size_t   elementLength = pDefaultArray->GetSlabLengths()[0];
                AQBlock1 *pEnd = pDefaultArray->BeginAt(0) + (pDefaultArray->GetDimensionLengths()[0] * elementLength);
                AQBlock1 *pElementData = pDefaultArray->BeginAt(0);

                for (; pElementData < pEnd; pElementData += elementLength)
                {
                    stringIndex = UnflattenData(pString, true, stringIndex, pElementData, elementType, null);
                    if (stringIndex == -1)
                        return -1;
                }
            }
            break;
        }
        case kEncoding_Cluster:
        {
            IntIndex count = type->SubElementCount();

            for (IntIndex j = 0; j < count; j++)
            {
                // Recursively call UnflattenData for each element with
                // prependArrayLength set to true.
                TypeRef elementType = type->GetSubElement(j);
                IntIndex offset = elementType->ElementOffset();
                AQBlock1* pElementData = pData ? (AQBlock1*)pData + offset : null;

                stringIndex = UnflattenData(pString, true, stringIndex, pDefaultData, elementType, pElementData);
                if (stringIndex == -1)
                    return -1;
            }
            break;
        }
        default:
        {
            Int32 copyLength = type->TopAQSize();

            // If the string is long enough, copy data.
            if (stringIndex + copyLength <= pString->Length())
            {
                if (pData)
                    memcpy(pData, pString->BeginAt(stringIndex), copyLength);
                stringIndex += copyLength;
            }
            else
                return -1;
            break;
        }
    }

    return stringIndex;
}

VIREO_FUNCTION_SIGNATURE7(UnflattenFromString, StringRef, Boolean, StaticType, void, StringRef, void, Boolean)
{
    StringRef pString = _Param(0);
    Boolean prependArrayLength = _ParamPointer(1) ? _Param(1) : true;
    TypeRef type = _ParamPointer(2);
    void *pDefaultData = _ParamPointer(3);
    StringRef pRemainder = _ParamPointer(4) ? _Param(4) : null;
    void *pData  = _ParamPointer(5);
    Boolean error;

    IntIndex remainderIndex = UnflattenData(pString, prependArrayLength, 0, pDefaultData, type, pData);
    IntIndex remainderLength = pString->Length() - remainderIndex;
    error = (remainderIndex == -1);

    if (error)
        type->CopyData(pDefaultData, pData);

    // Set the optional output param for the remaining string.
    if (pRemainder)
    {
        pRemainder->Resize1D(0);
        if (!error)
            pRemainder->Append(remainderLength, pString->BeginAt(remainderIndex));
    }

    // Set the optional output param for error.
    if (_ParamPointer(6))
        _Param(6) = error;

    return _NextInstruction();
}

DEFINE_VIREO_BEGIN(TDCodecLVFlat)
    DEFINE_VIREO_FUNCTION(FlattenToString, "p(i(.StaticTypeAndData) i(.Boolean) o(.String))");
    DEFINE_VIREO_FUNCTION(UnflattenFromString, "p(i(.String) i(.Boolean) i(.StaticTypeAndData) o(.String) o(.*) o(.Boolean))");
DEFINE_VIREO_END()

} // namespace Vireo
