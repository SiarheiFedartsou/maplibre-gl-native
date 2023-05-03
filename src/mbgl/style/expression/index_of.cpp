#include "mbgl/style/expression/expression.hpp"
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/index_of.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {
namespace style {
namespace expression {

EvaluationResult IndexOf::evaluate(const EvaluationContext &params) const {
  const EvaluationResult evaluatedKeyword = keyword->evaluate(params);
  const EvaluationResult evaluatedInput = input->evaluate(params);
  if (!evaluatedKeyword) {
    return evaluatedKeyword.error();
  }
  if (!evaluatedInput) {
    return evaluatedInput.error();
  }

  if (!(evaluatedKeyword->is<double>() || evaluatedKeyword->is<std::string>() ||
        evaluatedKeyword->is<bool>())) {
    return EvaluationError{"Expected double, string or boolean as keyword."};
  }

  const EvaluationResult evaluatedFromIndex = fromIndex->evaluate(params);
  if (!evaluatedFromIndex) {
    return evaluatedFromIndex.error();
  }
  int fromIndexValue = static_cast<int>(evaluatedFromIndex->get<double>());

  if (evaluatedInput->is<std::vector<Value>>()) {
    return evaluateForArrayInput(evaluatedInput->get<std::vector<Value>>(),
                                 *evaluatedKeyword, fromIndexValue);
  }
  if (evaluatedInput->is<std::string>()) {
    return evaluateForStringInput(evaluatedInput->get<std::string>(),
                                  *evaluatedKeyword, fromIndexValue);
  }

  return EvaluationError{"Expected array or string as input."};
}

bool IndexOf::validateFromIndex(int fromIndexValue, size_t maxIndex,
                                std::string *error) const {
  assert(error);
  if (fromIndexValue < 0) {
    *error = "Array index out of bounds: " + util::toString(fromIndexValue) +
             " < 0.";
    return false;
  }
  if (static_cast<size_t>(fromIndexValue) > maxIndex) {
    *error = "Array index out of bounds: " + util::toString(fromIndexValue) +
             " > " + util::toString(maxIndex) + ".";
    return false;
  }
  return true;
}

EvaluationResult IndexOf::evaluateForArrayInput(const std::vector<Value> &array,
                                                const Value &keywordValue,
                                                int fromIndexValue) const {
  std::string error;
  if (!validateFromIndex(fromIndexValue, array.size() - 1, &error)) {
    return EvaluationError{std::move(error)};
  }

  for (size_t index = static_cast<size_t>(fromIndexValue); index < array.size();
       ++index) {
    if (array[index] == keywordValue) {
      return static_cast<double>(index);
    }
  }
  return static_cast<double>(-1);
}

EvaluationResult IndexOf::evaluateForStringInput(const std::string &string,
                                                 const Value &keywordValue,
                                                 int fromIndexValue) const {
  std::string error;
  if (!validateFromIndex(fromIndexValue, string.size() - 1, &error)) {
    return EvaluationError{std::move(error)};
  }

  std::string keywordString;
  if (keywordValue.is<std::string>()) {
    keywordString = keywordValue.get<std::string>();
  } else if (keywordValue.is<bool>()) {
    keywordString = keywordValue.get<bool>() ? "true" : "false";
  } else {
    keywordString = util::toString(keywordValue.get<double>());
  }
  size_t index = string.find(keywordString, fromIndexValue);
  if (index == std::string::npos) {
    return static_cast<double>(-1);
  }
  return static_cast<double>(index);
}

void IndexOf::eachChild(
    const std::function<void(const Expression &)> &visit) const {
  visit(*keyword);
  visit(*input);
  visit(*fromIndex);
}

using namespace mbgl::style::conversion;
ParseResult IndexOf::parse(const Convertible &value, ParsingContext &ctx) {
  assert(isArray(value));

  std::size_t length = arrayLength(value);
  if (length != 3 && length != 4) {
    ctx.error("Expected 2 or 3 arguments, but found " +
              util::toString(length - 1) + " instead.");
    return ParseResult();
  }

  ParseResult keyword = ctx.parse(arrayMember(value, 1), 1);
  ParseResult input = ctx.parse(arrayMember(value, 2), 2);

  ParseResult fromIndex = ctx.parse(arrayMember(value, 3), 3, {type::Number});

  if (!keyword || !input)
    return ParseResult();

  return ParseResult(
      std::make_unique<IndexOf>(std::move(*keyword), std::move(*input),
                                fromIndex ? std::move(*fromIndex) : nullptr));
}

} // namespace expression
} // namespace style
} // namespace mbgl
