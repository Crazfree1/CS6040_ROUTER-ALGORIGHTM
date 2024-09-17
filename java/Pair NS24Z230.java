package network;

import java.util.HashSet;
import java.util.Set;

public class Pair {
    private final Integer first;
    private final Integer second;

    public Pair(Integer first, Integer second) {
        this.first = first;
        this.second = second;
    }

    public Integer getFirst() {
        return first;
    }

    public Integer getSecond() {
        return second;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o)
            return true;
        if (o == null || getClass() != o.getClass())
            return false;
        Pair pair = (Pair) o;
        return first.equals(pair.first) && second.equals(pair.second);
    }

    @Override
    public int hashCode() {
        return first.hashCode() + second.hashCode();
    }

    @Override
    public String toString() {
        return "(" + first + ", " + second + ")";
    }

    public static Set<Pair> findUniquePairs(Set<Integer> numbers) {
        Set<Pair> uniquePairs = new HashSet<>();
        Integer[] numArray = numbers.toArray(new Integer[0]);

        for (int i = 0; i < numArray.length; i++) {
            for (int j = i + 1; j < numArray.length; j++) {
                uniquePairs.add(new Pair(numArray[i], numArray[j]));
            }
        }

        return uniquePairs;
    }

    public Pair reverse() {
        return new Pair(second, first);
    }
}