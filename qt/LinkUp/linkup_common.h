#ifndef COMMON_H
#define COMMON_H

// 图片类型数目
#define ELEMENT_SIZE 12


// 游戏难度等级
typedef enum {
    PAR_LEVEL_0 = 0,
    PAR_LEVEL_1,
    PAR_LEVEL_2,
    PAR_LEVEL_3,
    PAR_LEVEL_END
} PAR_LEVEL_T;

// 游戏界面标识
typedef enum {
    UI_MAIN = 0,
    UI_SETTING,
    UI_HELP,
    UI_START,
    UI_EXIT
} UI_T;

// 游戏状态
typedef enum {
    GAME_PRE,
    GAME_START,
    GAME_PLAYING,
    GAME_OVER,
    GAME_END
} GAME_STATUS;

// 命令解析状态机
typedef enum {
    INT_PRE,
    INT_GETROW1,
    INT_GETCOL1,
    INT_GETP1,
    INT_GETROW2,
    INT_GETCOL2,
    INT_END
} INT_STATUS;

// 选中图片元素的状态
typedef enum {
    SEL_NONE,
    SEL_ONE,
    SEL_TWO,
    SEL_END
} SEL_STATUS;



#endif // COMMON_H

