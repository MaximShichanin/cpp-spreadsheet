#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>
#include <unordered_set>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    std::pair<Position, Position> GetLeftRightCorners() const;
    
    bool CheckForCircularDependencies(const CellInterface* cell, Position head) const;
    
private:
    struct position_hash { 
        size_t operator()(const Position& p) const;
    };
    
    std::unordered_map<Position, std::unique_ptr<CellInterface>, position_hash> data_;
};
