rm -vr ../../explorer_stm32f4/USER/gui/*

cp -vrp ../src/ -t ../../explorer_stm32f4/gui/

rm -vrf ../../explorer_stm32f4/gui/*/*.txt

echo ""
echo ""
echo "==== Enter any key exit... ===="
read key

