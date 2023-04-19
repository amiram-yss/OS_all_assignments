#!/bin/bash
#Amiram Yassif 314985474
declare -i position=0
declare -i pts1=50
declare -i pts2=50
rpts1=0
rpts2=0
winner=0

ongoing=1

print_field() {
    echo " Player 1: ${pts1}         Player 2: ${pts2} "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "

    case $position in
        -3)
            echo "O|       |       #       |       | "
            ;;
        -2)
            echo " |   O   |       #       |       | "
            ;;
        -1)
            echo " |       |   O   #       |       | "
            ;;
        0)
            echo " |       |       O       |       | "
            ;;
        1)
            echo " |       |       #   O   |       | "
            ;;
        2)
            echo " |       |       #       |   O   | "
            ;;
        3)
            echo " |       |       #       |       |O"
            ;;

    esac
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo " --------------------------------- "
}

user1_mov() {
    echo "PLAYER 1 PICK A NUMBER: "
    buffer=""
    read -s buffer
    # remove leading zeros
    buffer=$(echo $buffer | sed 's/^0*//')   
    while [[ !($buffer =~ ^[0-9]+$) 
                || $buffer -lt 0 
                || $buffer -gt $pts1 ]]
    do
        echo "NOT A VALID MOVE !"
        read -s buffer
    done
    pts1=$pts1-$buffer
    rpts1=$buffer
}

user2_mov() {
    echo "PLAYER 2 PICK A NUMBER: "
    buffer=""
    read -s buffer
    # remove leading zeros
    buffer=$(echo $buffer | sed 's/^0*//')   
    while [[ !($buffer =~ ^[0-9]+$) 
                || $buffer -lt 0 
                || $buffer -gt $pts2 ]]
    do
        echo "NOT A VALID MOVE !"
        read -s buffer
    done
    pts2=$pts2-$buffer
    rpts2=$buffer
}

update_position() {
    if [ $rpts1 -gt $rpts2 ]
    then
        #P1 wins round
        if [ $position -gt 0 ]
        then
            position=$[$position+1]
        else
            position=1
        fi

    elif [ $rpts2 -gt $rpts1 ]
    then
        #P1 wins round
        if [ $position -lt 0 ]
        then
            position=$[$position-1]
        else
            position=-1
        fi
    fi
}

print_last_move() {
    echo -e "       Player 1 played: ${rpts1}\n       Player 2 played: ${rpts2}\n\n"
}

check_game_over() {
    #If scored the ball into the goal :)
    if [ $position -eq 3 ]
    then
        ongoing=0
        winner=1
    fi
    if [ $position -eq -3 ]
    then
        ongoing=0
        winner=2
    fi
    # player 1 ran out of points.
    if [ $pts1 -eq 0 ]
    then
        ongoing=0
        #both player hit 0 at the same time check side of ball
        if [ $pts2 -eq 0 ]
        then
            if [ $position -gt 0 ]
            then
                winner=1
            elif [ $position -lt 0 ]
            then
                winner=2
            #if SOMEHOW the ball didnt move... DRAW
            else
                winner=0
            fi
        #player
        else
            winner=2
        fi
    elif [ $pts2 -eq 0 ]
    then
        ongoing=0
        winner=1
    fi
}

print_winner() {
    if [ $winner -eq 1 ]
    then
        position=3
    elif [ $winner -eq 2 ]
    then
        position=-3
    fi
    
    #print_field

    case $winner in
        0)
            echo "IT'S A DRAW !"
            ;;
        1)
            echo "PLAYER 1 WINS !"
            ;;
        2)
            echo "PLAYER 2 WINS !"
            ;;
    esac
}

#
print_field
user1_mov
user2_mov
update_position
check_game_over
#
while [ $ongoing -eq 1 ]
do
    print_field
    print_last_move
    user1_mov
    user2_mov
    update_position
    check_game_over
done
print_field
print_last_move
print_winner
