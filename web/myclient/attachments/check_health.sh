#!/bin/bash
if curl -i "http://localhost:10047/index.php" 2>&1 | grep "success" ; 
    then echo "health" 
    else date && docker-compose down && docker-compose up -d --build &>/dev/null && date
fi
