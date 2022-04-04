SIZE_OF_COLLECTION="$1"
NUMBER_OF_CORES=10

for iteration in $(seq 0 $(($NUMBER_OF_CORES - 1)));
do
	init="$(($iteration * $SIZE_OF_COLLECTION/$NUMBER_OF_CORES + 1))"
	end="$(($iteration * $SIZE_OF_COLLECTION/$NUMBER_OF_CORES + $SIZE_OF_COLLECTION/$NUMBER_OF_CORES))"
	./create_collection -i "$init" -e "$end" &
done
