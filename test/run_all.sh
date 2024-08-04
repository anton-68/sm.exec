#!/bin/bash

find ./ -maxdepth 1 -executable -not -wholename "./" -not -name "*.sh" |
while IFS= read -r COMMAND
do
        echo ""
        echo "" 
        echo "*******     "$COMMAND"     *******"
        echo ""
        echo ""
        $COMMAND > $COMMAND.log
done