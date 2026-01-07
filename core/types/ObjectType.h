#pragma once

// オブジェクトの種類を識別するための列挙型
enum class ObjectType {
    ROOT_STEP,
    ROOT_SIMULATION,
    ROOT_INFILL,
    ROOT_BC,
    ROOT_BC_CONSTRAINTS,
    ROOT_BC_LOADS, // Line 7
    
    ITEM_STEP,
    ITEM_SIMULATION,
    ITEM_INFILL_REGION,
    ITEM_BC_CONSTRAINT,
    ITEM_BC_LOAD,
    
    NONE // 選択なし用
};
