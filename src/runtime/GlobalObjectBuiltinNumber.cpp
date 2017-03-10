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

#include "Escargot.h"
#include "GlobalObject.h"
#include "Context.h"
#include "NumberObject.h"

namespace Escargot {

static int itoa(int64_t value, char* sp, int radix)
{
    char tmp[256]; // be careful with the length of the buffer
    char* tp = tmp;
    int i;
    uint64_t v;

    int sign = (radix == 10 && value < 0);
    if (sign)
        v = -value;
    else
        v = (uint64_t)value;

    while (v || tp == tmp) {
        i = v % radix;
        v /= radix; // v/=radix uses less CPU clocks than v=v/radix does
        if (i < 10)
            *tp++ = i + '0';
        else
            *tp++ = i + 'a' - 10;
    }

    int64_t len = tp - tmp;

    if (sign) {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp) {
        *sp++ = *--tp;
    }
    *sp++ = 0;

    return len;
}

static Value builtinNumberConstructor(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    NumberObject* numObj;
    if (isNewExpression) {
        numObj = thisValue.asPointerValue()->asObject()->asNumberObject();
        if (argc == 0)
            numObj->setPrimitiveValue(state, 0);
        else
            numObj->setPrimitiveValue(state, argv[0].toNumber(state));
        return numObj;
    } else {
        if (argc == 0)
            return Value(0);
        else
            return Value(argv[0].toNumber(state));
    }
}

static Value builtinNumberToFixed(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    double number = 0.0;

    if (thisValue.isNumber()) {
        number = thisValue.asNumber();
    } else if (thisValue.isPointerValue() && thisValue.asPointerValue()->isNumberObject()) {
        number = thisValue.asPointerValue()->asNumberObject()->primitiveValue();
    } else {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toFixed.string(), errorMessage_GlobalObject_ThisNotNumber);
    }

    if (argc == 0) {
        bool isInteger = (static_cast<int64_t>(number) == number);
        if (isInteger) {
            char buffer[256];
            itoa(static_cast<int64_t>(number), buffer, 10);
            return new ASCIIString(buffer);
        } else {
            return Value(round(number)).toString(state);
        }
    } else if (argc >= 1) {
        double digitD = argv[0].toNumber(state);
        if (digitD == 0 || std::isnan(digitD)) {
            return Value(round(number)).toString(state);
        }
        int digit = (int)trunc(digitD);
        if (digit < 0 || digit > 20) {
            ErrorObject::throwBuiltinError(state, ErrorObject::RangeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toFixed.string(), errorMessage_GlobalObject_RangeError);
        }
        if (std::isnan(number) || std::isinf(number)) {
            return Value(number).toString(state);
        } else if (std::abs(number) >= pow(10, 21)) {
            return Value(round(number)).toString(state);
        }

        std::basic_ostringstream<char> stream;
        if (number < 0)
            stream << "-";
        stream << "%." << digit << "lf";
        std::string fstr = stream.str();
        char buf[512];
        snprintf(buf, sizeof(buf), fstr.c_str(), std::abs(number));
        return Value(new ASCIIString(buf));
    }
    return Value();
}

static Value builtinNumberToExponential(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    double number = 0.0;

    if (thisValue.isNumber()) {
        number = thisValue.asNumber();
    } else if (thisValue.isPointerValue() && thisValue.asPointerValue()->isNumberObject()) {
        number = thisValue.asPointerValue()->asNumberObject()->primitiveValue();
    } else {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toExponential.string(), errorMessage_GlobalObject_ThisNotNumber);
    }

    int digit = 0; // only used when an argument is given
    if (argc > 0) {
        double fractionDigits = argv[0].toNumber(state);
        digit = (int)trunc(fractionDigits);
    }
    if (std::isnan(number)) { // 3
        return state.context()->staticStrings().NaN.string();
    }
    char buf[512];
    std::basic_ostringstream<char> stream;
    std::basic_ostringstream<char> expStream;
    if (number < 0) { // 5
        stream << "-";
        number = -1 * number;
    }
    if (std::isinf(number)) { // 6
        snprintf(buf, sizeof(buf), stream.str().c_str(), number, exp);
        StringBuilder builder;
        builder.appendString(buf);
        builder.appendString(state.context()->staticStrings().Infinity.string());
        return builder.finalize();
    }
    if (digit < 0 || digit > 20) {
        ErrorObject::throwBuiltinError(state, ErrorObject::RangeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toExponential.string(), errorMessage_GlobalObject_RangeError);
    }
    int exp = 0;
    if (number == 0) {
        exp = 0;
    } else if (std::abs(number) >= 10) {
        double tmp = number;
        while (tmp >= 10) {
            exp++;
            tmp /= 10.0;
        }
    } else if (std::abs(number) < 1) {
        double tmp = number;
        while (tmp < 1) {
            exp--;
            tmp *= 10.0;
        }
    }
    number /= pow(10, exp);
    if (argc == 0) {
        stream << "%.15lf";
    } else {
        stream << "%." << digit << "lf";
    }
    snprintf(buf, sizeof(buf), stream.str().c_str(), number);
    // remove trailing zeros
    char* tail = nullptr;
    if (argc == 0) {
        tail = buf + strlen(buf) - 1;
        while (*tail == '0' && *tail-- != '.') {
        }
        tail++;
    } else {
        for (size_t i = 0; i < strlen(buf); i++) {
            tail = &buf[i];
            if (*tail == '.') {
                break;
            }
        }
        tail++;
        for (int i = 0; i < digit; i++)
            tail++;
    }
    if (*(tail - 1) == '.')
        tail--;
    expStream << "e";
    if (exp >= 0) {
        expStream << "+";
    }
    expStream << "%d";
    snprintf(tail, 512 - (ptrdiff_t)(buf - tail), expStream.str().c_str(), exp);
    return Value(new ASCIIString(buf));
}

static Value builtinNumberToPrecision(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    double number = 0.0;

    if (thisValue.isNumber()) {
        number = thisValue.asNumber();
    } else if (thisValue.isPointerValue() && thisValue.asPointerValue()->isNumberObject()) {
        number = thisValue.asPointerValue()->asNumberObject()->primitiveValue();
    } else {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toPrecision.string(), errorMessage_GlobalObject_ThisNotNumber);
    }

    if (argc == 0 || argv[0].isUndefined()) {
        return Value(number).toString(state);
    } else if (argc >= 1) {
        double x = number;
        double p_d = argv[0].toNumber(state);
        if (std::isnan(x)) {
            return state.context()->staticStrings().NaN.string();
        }
        std::basic_ostringstream<char> stream;
        if (x < 0) {
            stream << "-";
            x = -x;
        }
        if (std::isinf(x)) {
            stream << "Infinity";
        } else {
            int p = (int)trunc(p_d);
            if (p < 1 || p > 21) {
                ErrorObject::throwBuiltinError(state, ErrorObject::RangeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toPrecision.string(), errorMessage_GlobalObject_RangeError);
            }
            if (LIKELY(x != 0)) {
                int log10_num = trunc(log10(x));
                if (log10_num + 1 <= p && log10_num > -6) {
                    if (std::abs(x) >= 1) {
                        stream << "%" << log10_num + 1 << "." << (p - log10_num - 1) << "lf";
                    } else {
                        stream << "%" << log10_num << "." << (p - log10_num) << "lf";
                    }
                } else {
                    x = x / pow(10, log10_num);
                    if (std::abs(x) < 1) {
                        x *= 10;
                        log10_num--;
                    }
                    stream << "%1." << (p - 1) << "lf"
                           << "e" << ((log10_num >= 0) ? "+" : "") << log10_num;
                }
            } else {
                stream << "%1." << (p - 1) << "lf";
            }
        }
        std::string fstr = stream.str();
        char buf[512];
        snprintf(buf, sizeof(buf), fstr.c_str(), x);
        return Value(new ASCIIString(buf));
    }
    return Value();
}

static Value builtinNumberToString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    double number = 0.0;

    if (thisValue.isNumber()) {
        number = thisValue.asNumber();
    } else if (thisValue.isPointerValue() && thisValue.asPointerValue()->isNumberObject()) {
        number = thisValue.asPointerValue()->asNumberObject()->primitiveValue();
    } else {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toString.string(), errorMessage_GlobalObject_ThisNotNumber);
    }

    if (std::isnan(number) || std::isinf(number)) {
        return (Value(number).toString(state));
    }
    double radix = 10;
    if (argc > 0 && !argv[0].isUndefined()) {
        radix = argv[0].toInteger(state);
        if (radix < 2 || radix > 36)
            ErrorObject::throwBuiltinError(state, ErrorObject::RangeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toString.string(), errorMessage_GlobalObject_RadixInvalidRange);
    }
    if (radix == 10)
        return (Value(number).toString(state));
    else {
        bool isInteger = (static_cast<int64_t>(number) == number);
        if (isInteger) {
            bool minusFlag = (number < 0) ? 1 : 0;
            number = (number < 0) ? (-1 * number) : number;
            char buffer[256];
            if (minusFlag) {
                buffer[0] = '-';
                itoa(static_cast<int64_t>(number), &buffer[1], radix);
            } else {
                itoa(static_cast<int64_t>(number), buffer, radix);
            }
            return (new ASCIIString(buffer));
        } else {
            ASSERT(Value(number).isDouble());
            NumberObject::RadixBuffer s;
            const char* str = NumberObject::toStringWithRadix(state, s, number, radix);
            return new ASCIIString(str);
        }
    }
    return Value();
}

static Value builtinNumberToLocaleString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    RESOLVE_THIS_BINDING_TO_OBJECT(thisObject, Number, toLocaleString);


    ObjectGetResult toStrFuncGetResult = thisObject->get(state, ObjectPropertyName(state.context()->staticStrings().toString));
    if (toStrFuncGetResult.hasValue()) {
        Value toStrFunc = toStrFuncGetResult.value(state, thisObject);
        if (toStrFunc.isFunction()) {
            return FunctionObject::call(state, toStrFunc, thisObject, argc, argv, isNewExpression);
        }
    }
    ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().Number.string(), true, state.context()->staticStrings().toLocaleString.string(), errorMessage_GlobalObject_ToLocaleStringNotCallable);
    RELEASE_ASSERT_NOT_REACHED();
}

static Value builtinNumberValueOf(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
    if (thisValue.isNumber()) {
        return Value(thisValue);
    } else if (thisValue.isObject() && thisValue.asObject()->isNumberObject()) {
        return Value(thisValue.asPointerValue()->asNumberObject()->primitiveValue());
    }
    ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, errorMessage_GlobalObject_ThisNotNumber);
    RELEASE_ASSERT_NOT_REACHED();
}

void GlobalObject::installNumber(ExecutionState& state)
{
    const StaticStrings* strings = &state.context()->staticStrings();
    m_number = new FunctionObject(state, NativeFunctionInfo(strings->Number, builtinNumberConstructor, 1, [](ExecutionState& state, size_t argc, Value* argv) -> Object* {
                                      return new NumberObject(state);
                                  }),
                                  FunctionObject::__ForBuiltin__);
    m_number->markThisObjectDontNeedStructureTransitionTable(state);
    m_number->setPrototype(state, m_functionPrototype);
    m_numberPrototype = m_objectPrototype;
    m_numberPrototype = new NumberObject(state, 0);
    m_numberPrototype->markThisObjectDontNeedStructureTransitionTable(state);
    m_numberPrototype->setPrototype(state, m_objectPrototype);
    m_number->setFunctionPrototype(state, m_numberPrototype);
    m_numberPrototype->defineOwnProperty(state, ObjectPropertyName(strings->constructor), ObjectPropertyDescriptor(m_number, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_numberPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(strings->toString),
                                                        ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(strings->toString, builtinNumberToString, 1, nullptr, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_numberPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(strings->toLocaleString),
                                                        ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(strings->toLocaleString, builtinNumberToLocaleString, 0, nullptr, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_numberPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(strings->toFixed),
                                                        ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(strings->toFixed, builtinNumberToFixed, 1, nullptr, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_numberPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(strings->toExponential),
                                                        ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(strings->toExponential, builtinNumberToExponential, 1, nullptr, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_numberPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(strings->toPrecision),
                                                        ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(strings->toPrecision, builtinNumberToPrecision, 1, nullptr, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    // $20.1.3.26 Number.prototype.valueOf
    m_numberPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(strings->valueOf),
                                                        ObjectPropertyDescriptor(new FunctionObject(state, NativeFunctionInfo(strings->valueOf, builtinNumberValueOf, 0, nullptr, NativeFunctionInfo::Strict)),
                                                                                 (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));


    m_number->setFunctionPrototype(state, m_numberPrototype);

    ObjectPropertyDescriptor::PresentAttribute allFalsePresent = (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::NonWritablePresent
                                                                                                              | ObjectPropertyDescriptor::NonEnumerablePresent
                                                                                                              | ObjectPropertyDescriptor::NonConfigurablePresent);
    // $20.1.2.6 Number.MAX_SAFE_INTEGER
    m_number->defineOwnPropertyThrowsException(state, strings->MAX_SAFE_INTEGER, ObjectPropertyDescriptor(Value(9007199254740991.0), (ObjectPropertyDescriptor::PresentAttribute)allFalsePresent));
    // $20.1.2.7 Number.MAX_VALUE
    m_number->defineOwnPropertyThrowsException(state, strings->MAX_VALUE, ObjectPropertyDescriptor(Value(1.7976931348623157E+308), (ObjectPropertyDescriptor::PresentAttribute)allFalsePresent));
    // $20.1.2.8 Number.MIN_SAFE_INTEGER
    m_number->defineOwnPropertyThrowsException(state, strings->MIN_SAFE_INTEGER, ObjectPropertyDescriptor(Value(-9007199254740991.0), (ObjectPropertyDescriptor::PresentAttribute)allFalsePresent));
    // $20.1.2.9 Number.MIN_VALUE
    m_number->defineOwnPropertyThrowsException(state, strings->MIN_VALUE, ObjectPropertyDescriptor(Value(5E-324), allFalsePresent));
    // $20.1.2.10 Number.NaN
    m_number->defineOwnPropertyThrowsException(state, strings->NaN, ObjectPropertyDescriptor(Value(std::numeric_limits<double>::quiet_NaN()), allFalsePresent));
    // $20.1.2.11 Number.NEGATIVE_INFINITY
    m_number->defineOwnPropertyThrowsException(state, strings->NEGATIVE_INFINITY, ObjectPropertyDescriptor(Value(-std::numeric_limits<double>::infinity()), allFalsePresent));
    // $20.1.2.14 Number.POSITIVE_INFINITY
    m_number->defineOwnPropertyThrowsException(state, strings->POSITIVE_INFINITY, ObjectPropertyDescriptor(Value(std::numeric_limits<double>::infinity()), allFalsePresent));

    defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().Number),
                      ObjectPropertyDescriptor(m_number, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
}
}
