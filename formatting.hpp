/*  =====================================================   Text Formatting   ========================================================  */
/*  =====================================================          &          ========================================================  */
/*  ===================================================== Cursor Manipulation ========================================================  */

#include <string>

const std::string COLOR_RED = "31";
const std::string COLOR_GREEN = "32";
const std::string COLOR_YELLOW = "33";
const std::string COLOR_BLUE = "34";
const std::string COLOR_PURPLE = "35";
const std::string COLOR_CYAN = "36";

const std::string MOVE_UP = "A";
const std::string MOVE_DOWN = "B";
const std::string MOVE_RIGHT = "C";
const std::string MOVE_LEFT = "D";

std::string REMOVE_ALL_FORMATTING()
{
    return "\033[0m";
}

/// @brief Saves the current cursor position
/// @return Corresponding octal ANSI escape code
std::string SAVE_CURSOR()
{
    return "\033[s";
}

/// @brief Restores the cursor to the last saved position
/// @return Corresponding octal ANSI escape code
std::string RESTORE_CURSOR()
{
    return "\033[u";
}

/// @brief Sets the color of the following text to __color
/// @return Corresponding octal ANSI escape code
std::string SET_COLOR(std::string __color)
{
    return "\033[" + __color + "m";
}

/// @brief Moves the cursor '__steps' steps in '__direction' direction
/// @param __steps number of steps to be moved
/// @param __direction direction of movement
/// @return Corresponding octal ANSI escape code
std::string MOVE_CURSOR(int __steps, const std::string __direction)
{
    if( __steps > 0 )
        return "\033[" + std::to_string( __steps ) + __direction;
    return "";
}

/// @brief Navigates the cursor from its current position downwards.
/// @param __row number of rows from current point downwards
/// @param __column number of columns from current point to the right
/// @return Corresponding octal ANSI escape code
std::string NAVIGATE_CURSOR(int __row, int __column)
{
    int horizontal_distance = __column;
    if( __column == 2 ) horizontal_distance = 16;
    else if( __column == 3 ) horizontal_distance = 26;

    std::cout << RESTORE_CURSOR();
    fflush( stdout );
    std::cout << MOVE_CURSOR( __row, MOVE_DOWN );
    fflush( stdout );
    std::cout << MOVE_CURSOR( horizontal_distance, MOVE_RIGHT );
    fflush( stdout );

    return "" + RESTORE_CURSOR() + MOVE_CURSOR( __row, MOVE_DOWN ) + MOVE_CURSOR( horizontal_distance, MOVE_RIGHT );
}