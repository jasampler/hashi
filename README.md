# hashi
Console program to find the solutions of any Hashiwokakero puzzle.

The input must be digits and dots separated by newlines or slashes.

For example, an example of valid input is:

    2 . . 3 . 1 .
    . . . . 3 . 2
    . 2 . 4 . . .
    4 . 2 . 2 . .
    . 1 . 3 . . 3
    2 . 2 . 1 . .
    . 2 . 3 . . 2

Which can also be written as:

    2003010/0000302/0204000/4020200/0103003/2020100/0203002

The program shows the empty board and then all valid solutions of the puzzle, for example:

    (2)   -    -   (3)   -   (1)   .   
                                       
     '    .    .    '   (3)   -   (2)  
                                       
     '   (2)   -   (4)   '    .    '   
                                       
    (4)   +   (2)   +   (2)   .    '   
                                       
     '   (1)   +   (3)   +    -   (3)  
                                       
    (2)   +   (2)   +   (1)   .    '   
                                       
     .   (2)   -   (3)   -    -   (2)  


    (2)------------(3)-------(1)   .   
     !              !                  
     !    .    .    !   (3)-------(2)  
     !              !    !!        !   
     !   (2)=======(4)   !!   .    !   
     !              !    !!        !   
    (4)=======(2)   !   (2)   .    !   
     !              !              !   
     !   (1)-------(3)------------(3)  
     !                             !   
    (2)-------(2)-------(1)   .    !   
                                   !   
     .   (2)=======(3)------------(2)  

This program is dedicated to my self of the past, who tried to solve it in Java many years ago and failed because it was not so easy as it seemed.

Enjoy!
