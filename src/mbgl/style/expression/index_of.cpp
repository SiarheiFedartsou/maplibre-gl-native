#include "mbgl/style/expression/expression.hpp"
#include <mbgl/style/expression/index_of.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/util/string.hpp>


namespace mbgl {
namespace style {
namespace expression {

EvaluationResult IndexOf::evaluate(const EvaluationContext& params) const {
    const EvaluationResult evaluatedKeyword = keyword->evaluate(params);
    const EvaluationResult evaluatedInput = input->evaluate(params);
    if (!evaluatedKeyword) {
        return evaluatedKeyword.error();
    }
    if (!evaluatedInput) {
        return evaluatedInput.error();
    }


    const auto inputArray = evaluatedInput->get<std::vector<Value>>();

    int fromIndexValue = 0;
    if (fromIndex) {
        const EvaluationResult evaluatedFromIndex = fromIndex->evaluate(params);
        if (!evaluatedFromIndex) {
            return evaluatedFromIndex.error();
        }
        fromIndexValue = static_cast<int>(evaluatedFromIndex->get<double>());

        if (fromIndexValue > 0) {
            return EvaluationError {
                "Array index out of bounds: " + util::toString(fromIndexValue) + " < 0."
        };
        }
        if (static_cast<size_t>(fromIndexValue) >= inputArray.size()) {
        return EvaluationError {
            "Array index out of bounds: " + util::toString(fromIndexValue) +
            " > " + util::toString(inputArray.size() - 1) + "."
        };
    }
    }

    
    for (size_t index = static_cast<size_t>(fromIndexValue); index < inputArray.size(); ++index) {
        if (inputArray[index] == *evaluatedKeyword) {
            return static_cast<double>(index);
        }
    }
    return static_cast<double>(-1);
}

void IndexOf::eachChild(const std::function<void(const Expression&)>& visit) const {
    visit(*keyword);
    visit(*input);
    visit(*fromIndex);
}

using namespace mbgl::style::conversion;
ParseResult IndexOf::parse(const Convertible& value, ParsingContext& ctx) {
    assert(isArray(value));

    std::size_t length = arrayLength(value);
    if (length != 3 && length != 4) {
        ctx.error("Expected 2 or 3 arguments, but found " + util::toString(length - 1) + " instead.");
        return ParseResult();
    }

    ParseResult keyword = ctx.parse(arrayMember(value, 1), 1, {type::Value});

    type::Type inputType = type::Array(ctx.getExpected() ? *ctx.getExpected() : type::Value);
    ParseResult input = ctx.parse(arrayMember(value, 2), 2, {inputType});


    ParseResult fromIndex = ctx.parse(arrayMember(value, 3), 3, {type::Number});
    

    if (!keyword || !input) return ParseResult();



    return ParseResult(std::make_unique<IndexOf>(std::move(*keyword), std::move(*input), fromIndex ? std::move(*fromIndex) : nullptr));

}

} // namespace expression
} // namespace style
} // namespace mbgl
