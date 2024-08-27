(ns aoc23.d2
  (:require [aoc23.string :as s]))

(defn line-is-possible [line]
  (let [s (-> line s/getnext (s/trim \,) (s/trim \;) s/getnext)]
    (if (nil? s) true
        (let [next-num (s/getnum s)
              s (-> s s/matchnum s/getnext)]
          (cond (s/checkstr s "red")
            (if (> next-num 12) false
              (recur (s/matchstr s "red")))
            (s/checkstr s "green")
                (if (> next-num 13) false
                  (recur (s/matchstr s "green")))
            (s/checkstr s "blue")
                (if (> next-num 14) false
                  (recur (s/matchstr s "blue")))
            :else true
            )))))

(defn line-power [line]
  (let [line (-> line
                 (s/matchstr "Game")
                 s/getnext s/matchnum (s/trim \:))
        anc (fn [s max-map]
              (if (nil? s) (* (:red max-map) (:green max-map)
                              (:blue max-map))
                  (let [
                        s1 (-> s s/getnext (s/trim \,) (s/trim \;) s/getnext)
                        next-num (s/getnum s1)
                        s (-> s1 s/matchnum s/getnext)]
                    ;; (println "s1: " (first s1) "s: " (first s))
                    (cond (s/checkstr s "red")
                          (recur (s/matchstr s "red")
                                 (assoc max-map :red
                                        (max next-num (:red max-map))))
                          (s/checkstr s "green")
                          (recur (s/matchstr s "green")
                                 (assoc max-map :green
                                        (max next-num
                                             (:green max-map))))
                          (s/checkstr s "blue")
                          (recur (s/matchstr s "blue")
                                 (assoc max-map :blue
                                        (max next-num
                                             (:blue max-map))))
                      :else (* (:red max-map) (:green max-map)
                              (:blue max-map))
                      ))))]
    (anc line {:red 0 :green 0 :blue 0})))

(defn line-sum [line]
  (let [line (-> line (s/matchstr "Game") s/getnext)
        line-index (s/getnum line)]
    (if (line-is-possible (-> line s/matchnum (s/trim \:)))
      line-index 0)
    ))
(defn part-1 [lines]
  (->> lines (map line-sum) (reduce + 0)))
(defn part-2 [lines]
  (->> lines (map line-power) (reduce + 0)))
