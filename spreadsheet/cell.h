#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
private:
    struct CellCache {
        Value val_;
        bool modification_flag_ = true;
        
        operator bool() const;
        operator Value() const;
    };
    
    bool IsModified() const;
    void SetCache(Value&& val) const;

public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();
    
    Value GetValue() const override;
    std::string GetText() const override;
    
    std::vector<Position> GetReferencedCells() const override;
    
private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    
    mutable CellCache cache_;
};
