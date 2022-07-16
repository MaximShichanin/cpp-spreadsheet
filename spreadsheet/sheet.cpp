#include "sheet.h"

#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("wrong position"s);
    }
    auto temp_cell = std::move(std::make_unique<Cell>(*this));
    temp_cell->Set(text);
    if(CheckForCircularDependencies(temp_cell.get(), pos)) {
        throw CircularDependencyException("Circular dependency"s);
    }
    data_[pos] = std::move(temp_cell);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("wrong position"s);
    }
    auto iter = data_.find(pos);
    if(iter != data_.end()) {
        return data_.at(pos).get();
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("wrong position"s);
    }
    auto iter = data_.find(pos);
    if(iter != data_.end()) {
        return data_.at(pos).get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if(pos.IsValid()) {
       data_.erase(pos);
    }
    else {
        throw InvalidPositionException("wrong position"s);
    }
}

Size Sheet::GetPrintableSize() const {
    if(data_.empty()) {
        return {0, 0};
    }
    auto left_right = GetLeftRightCorners();
    return {left_right.second.row - left_right.first.row + 1,
            left_right.second.col - left_right.first.col + 1};
}

struct ValueToStringVisitor {
    std::string operator() (std::string str) const {
        return str;
    }
    std::string operator() (double d) const {
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    std::string operator() (FormulaError fe) const {
        std::ostringstream oss;
        oss << fe;
        return oss.str();
    }
};

void Sheet::PrintValues(std::ostream& output) const {
    if(data_.empty()) {
        return;
    }
    Position left_corner{};
    Position right_corner{};
    std::tie(left_corner, right_corner) = GetLeftRightCorners();
    for(int i = left_corner.row; i <= right_corner.row; ++i) {
        for(int j = left_corner.col; j <= right_corner.col; ++j) {
            auto current_cell_ptr = GetCell({i, j});
            if(current_cell_ptr) {
                output << std::visit(ValueToStringVisitor(), current_cell_ptr->GetValue());
            }
            if(j < right_corner.col) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    if(data_.empty()) {
        return;
    }
    Position left_corner{};
    Position right_corner{};
    std::tie(left_corner, right_corner) = GetLeftRightCorners();
    for(int i = left_corner.row; i <= right_corner.row; ++i) {
        for(int j = left_corner.col; j <= right_corner.col; ++j) {
            auto current_cell_ptr = GetCell({i, j});
            if(current_cell_ptr) {
                output << current_cell_ptr->GetText();
            }
            if(j < right_corner.col) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

size_t Sheet::position_hash::operator() (const Position& p) const {
    return std::hash<int>()(p.row) ^ std::hash<int>()(p.col);
}

std::pair<Position, Position> Sheet::GetLeftRightCorners() const {
    if(data_.empty()) {
        //return {Position::NONE, Position::NONE};
        return {{0,0}, {0, 0}};
    }
    else if(data_.size() == 1u) {
        auto pos = data_.begin()->first;
        return {{0, 0}, pos};
    }
    //Position left = Position::NONE;
    Position right = Position::NONE;
    for(const auto& [key, _] : data_) {
        //left.col = left.col == -1 || key.col < left.col ? key.col : left.col;
        //left.row = left.row == -1 || key.row < left.row ? key.row : left.row;
        right.col = right.col < key.col ? key.col : right.col;
        right.row = right.row < key.row ? key.row : right.row;
    }
    return {{0,0}, right};
}

bool Sheet::CheckForCircularDependencies(const CellInterface* cell, Position head) const {
    if(!cell) {
        return false;
    }
    bool res = false;
    for(const auto& next_cell_pos : cell->GetReferencedCells()) {
        if(next_cell_pos == head) {
            return true;
        }
        res = res || CheckForCircularDependencies(GetCell(next_cell_pos), head);
    }
    return res;
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
