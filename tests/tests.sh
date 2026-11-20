#!/bin/sh

cd tests || exit 2
BIN=../minimake
OK=0
KO=0

test_case()
{
    numero=$1
    attendu=$2
    shift 2

    "$BIN" "$@" > sortie.txt 2> erreur.txt
    code=$?

    bon=1
    [ "$code" -eq "$attendu" ] || bon=0
    cmp -s "resultats/attendu$numero.txt" sortie.txt || bon=0

    if [ "$attendu" -eq 0 ]; then
        [ ! -s erreur.txt ] || bon=0
    else
        [ -s erreur.txt ] || bon=0
    fi

    if [ "$bon" -eq 1 ]; then
        echo "[PASS] test$numero"
        OK=$((OK + 1))
    else
        echo "[FAIL] test$numero (code attendu $attendu, obtenu $code)"
        KO=$((KO + 1))
    fi
}
rm -f depA depB input.txt output.txt existing.txt source.txt
rm -f result.txt generated.txt should_not_exist sortie.txt erreur.txt

test_case 1 0 -f Makefile1
test_case 2 0 -f Makefile2
test_case 3 0 -f Makefile3
test_case 4 0 -f Makefile4
test_case 5 0 -f Makefile5
test_case 6 0 -f Makefile6
test_case 7 0 -f Makefile7 second
test_case 8 0 -f Makefile8
touch depA depB
test_case 9 0 -f Makefile9
test_case 10 0 -f Makefile10
test_case 11 0 -f Makefile11
test_case 12 0 -f Makefile12
test_case 13 0 -f Makefile13
test_case 14 0 -f Makefile14
test_case 15 0 -f Makefile15
test_case 16 0 -f Makefile16
test_case 17 0 -f Makefile17
test_case 18 2 -f Makefile18
test_case 19 2 -f Makefile19
test_case 20 2 -f Makefile20
test_case 21 0 -f Makefile21
test_case 22 0 -f Makefile22
test_case 23 0 -f Makefile23
test_case 24 0 -f Makefile24
test_case 25 0 -f Makefile25 one two
test_case 26 0 -f Makefile26
test_case 27 0 --file=Makefile27
test_case 28 0 --makefile=Makefile28
test_case 29 0 -h
test_case 30 0 -f Makefile30 -p
test_case 31 0 -f Makefile31 -p
touch input.txt output.txt
touch -t 202001010000 input.txt
touch -t 202101010000 output.txt
test_case 32 0 -f Makefile32
touch -t 202101010000 input.txt
touch -t 202001010000 output.txt
test_case 33 0 -f Makefile33
touch existing.txt
test_case 34 0 -f Makefile34
test_case 35 2 -f Makefile35 missing_target
test_case 36 2 -f Makefile36
test_case 37 0 -f Makefile37
touch source.txt
test_case 38 0 -f Makefile38
test_case 39 0 -f Makefile39
test_case 40 0 -f Makefile40
test_case 41 2 -f Makefile41
test_case 42 0 -f Makefile42
test_case 43 0 -f Makefile43

echo ""
echo "Resultat : $OK/43 tests passes, $KO echoues"

rm -f depA depB input.txt output.txt existing.txt source.txt
rm -f result.txt generated.txt should_not_exist sortie.txt erreur.txt
[ "$KO" -eq 0 ]
