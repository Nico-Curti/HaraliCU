#include <iostream>
#include <stdio.h>
#include <cstring>
#include "ImageData.h"

uint32_t ImageData::getRows() const{
    return rows;
}

uint32_t ImageData::getColumns() const{
    return columns;
}

int ImageData::getBorderSize() const {
    return appliedBorders;
}
uint32_t ImageData::getMaxGrayLevel() const{
    return maxGrayLevel;
}
uint32_t ImageData::getMinGrayLevel() const{
    return minGrayLevel;
}