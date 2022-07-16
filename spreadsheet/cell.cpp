#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;

class Cell::Impl {
public:
    virtual ~Impl() = default;
    
    virtual Cell::Value GetValue(SheetInterface& sheet) const = 0;
    virtual std::string GetString() const = 0;
    virtual std::vector<Position> GetReferencedCells() const {
        return {};
    }
};

class Cell::EmptyImpl final : public Cell::Impl {
public:
    EmptyImpl() = default;
    
    Cell::Value GetValue(SheetInterface& /*sheet*/) const override {
        return ""s;
    }
    
    std::string GetString() const override {
        return ""s;
    }
};

class Cell::TextImpl final : public Cell::Impl {
public:
    explicit TextImpl(std::string text) 
        : data_(std::move(text)) {}
        
    Cell::Value GetValue(SheetInterface& /*sheet*/) const override {
        return data_;
    }
    
    std::string GetString() const override {
        return data_;
    }
private:
    const std::string data_;
};

class Cell::FormulaImpl final : public Cell::Impl {
private:
    struct FormulaVisitor {
        Cell::Value operator() (double d) {
            return Cell::Value{d};
        }
        Cell::Value operator() (FormulaError fe) {
            return Cell::Value{fe};
        }
    };
public:
    explicit FormulaImpl(std::string formula) 
        : data_(std::move(ParseFormula(std::move(formula))))
    {
    }
    
    Cell::Value GetValue(SheetInterface& sheet) const override {
        return std::visit(FormulaVisitor(), data_->Evaluate(sheet));
    }
    
    std::string GetString() const override {
        return "="s.append(data_->GetExpression());
    }
    
    std::vector<Position> GetReferencedCells() const override {
        return data_->GetReferencedCells();
    }
private:
    std::unique_ptr<FormulaInterface> data_;
};
// Реализуйте следующие методы
Cell::Cell(Sheet& sheet) 
    : impl_(std::make_unique<EmptyImpl>(EmptyImpl{})) 
    , sheet_(sheet) {}

Cell::~Cell() = default;

bool Cell::IsModified() const {
    return cache_;
}

void Cell::SetCache(Value&& val) const {
    cache_.val_ = std::forward<Value>(val);
    cache_.modification_flag_ = false;
}

void Cell::Set(std::string text) {
    if(text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    }
    else if(text.front() == '=' && text.size() > 1u) {
        try {
            impl_ = std::make_unique<FormulaImpl>(FormulaImpl{text.substr(1u)});
            for(const auto& ref_cell : impl_->GetReferencedCells()) {
                auto cell_ptr = sheet_.GetCell(ref_cell);
                if(!cell_ptr) {
                    sheet_.SetCell(ref_cell, ""s);
                }
            }
        }
        catch(...) {
            throw FormulaException{"Unable to parse: "s.append(text)};
        }
    }
    else {
        impl_ = std::make_unique<TextImpl>(TextImpl{text});
    }
    cache_.modification_flag_ = true;
}

void Cell::Clear() {
    impl_.reset(nullptr);
    cache_.modification_flag_ = true;
}

struct ValueVisitor {
    Cell::Value operator() (std::string str) const {
        return str.front() == '\'' ? str.substr(1u) : str;
    }
    Cell::Value operator() (double d) const {
        return d;
    }
    Cell::Value operator() (FormulaError fe) const {
        return fe;
    }
};

Cell::Value Cell::GetValue() const {
    const auto& dependency = GetReferencedCells();
    if(!cache_ || std::any_of(dependency.begin(), dependency.end(), 
        [this] (const auto& pos) {
                                     const Cell* cell_ptr_ = reinterpret_cast<const Cell*>(sheet_.GetCell(pos));
                                     return cell_ptr_->IsModified();
                                 })) {
        
        auto value = impl_ ? std::visit(ValueVisitor(), impl_->GetValue(sheet_)) : Cell::Value{};
        SetCache(std::move(value));
    }
    return cache_;
}

std::string Cell::GetText() const {
    if(impl_) {
        return impl_->GetString();
    }
    return {};
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

Cell::CellCache::operator bool() const {
    return !modification_flag_;
}

Cell::CellCache::operator Value() const {
    return val_;
}
