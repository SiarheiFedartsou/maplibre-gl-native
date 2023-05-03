#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/style/conversion.hpp>
#include <memory>

namespace mbgl {
namespace style {
namespace expression {

class IndexOf : public Expression {
public:
    IndexOf(std::unique_ptr<Expression> keyword_, std::unique_ptr<Expression> input_, std::unique_ptr<Expression> fromIndex_) :
        Expression(Kind::IndexOf, input_->getType().get<type::Array>().itemType),
        keyword(std::move(keyword_)),
        input(std::move(input_)),
        fromIndex(std::move(fromIndex_))
    {}
    
    static ParseResult parse(const mbgl::style::conversion::Convertible& value, ParsingContext& ctx);
    
    EvaluationResult evaluate(const EvaluationContext& params) const override;
    void eachChild(const std::function<void(const Expression&)>&) const override;

    bool operator==(const Expression& e) const override {
        if (e.getKind() == Kind::IndexOf) {
            auto rhs = static_cast<const IndexOf*>(&e);
            const auto fromIndexEqual = (fromIndex && rhs->fromIndex && *fromIndex == *(rhs->fromIndex)) 
                || (!fromIndex && !rhs->fromIndex);
            return *keyword == *(rhs->keyword) && *input == *(rhs->input) && fromIndexEqual;
        }
        return false;
    }

    std::vector<std::optional<Value>> possibleOutputs() const override {
        return { std::nullopt };
    }
    
    std::string getOperator() const override { return "index-of"; }
private:
    EvaluationResult evaluateForArrayInput(const std::vector<Value>& array, const Value& keyword, int fromIndex) const;
    EvaluationResult evaluateForStringInput(const std::string& string, const Value& keyword, int fromIndex) const;
    
    bool validateFromIndex(int fromIndex, size_t maxIndex, std::string* error) const;

    size_t getFromIndex(const EvaluationContext& params, size_t maxIndex, std::string* error) const;
private:
    std::unique_ptr<Expression> keyword;
    std::unique_ptr<Expression> input;
    std::unique_ptr<Expression> fromIndex;
    
};

} // namespace expression
} // namespace style
} // namespace mbgl
