(ns aoc23.core
  (:require [aoc23.d1 :as d1]
            [aoc23.d2 :as d2]
            [aoc23.d3 :as d3]
            [clojure.string :as s])
  (:gen-class))

(defn split-file-into-lines
  [filename]
  (let [file (slurp filename)]
    (s/split-lines file)))

(defn print-usage
  []
  (println "Usage: lein run [day-number] [<part>1|2] [input-path]"))

(defn d1
  "Day 1"
  [argv]
  (println "Day 1")
  (println "part:" (first argv))
  (println "input:" (second argv))
  (let [input (split-file-into-lines (second argv))
        part (first argv)]
    (cond (= part "1")
          (let [output (d1/part-1 input)]
            (println "output: " output))
          (= part "2")
          (let [output (d1/part-2 input)]
            (println "output: " output))
            ;; (spit "output.txt" (d1/prep-output output)))
          :else (print-usage))))

(defn d2
  "Day 2"
  [argv]
  (println "Day 2")
  (println "part:" (first argv))
  (println "input:" (second argv))
  (let [input (split-file-into-lines (second argv))
        part (first argv)]
    (cond (= part "1")
          (let [output (d2/part-1 input)]
            (println "output: " output))
          (= part "2")
          (let [output (d2/part-2 input)]
            (println "output: " output))
          :else (print-usage))))

(defn d3
  "Day 3"
  [argv]
  (println "Day 3")
  (println "part:" (first argv))
  (println "input:" (second argv))
  (let [input (split-file-into-lines (second argv))
        part (first argv)]
    (cond (= part "1")
          (let [output (d2/part-1 input)]
            (println "output: " output))
          (= part "2")
          (let [output (d2/part-2 input)]
            (println "output: " output))
          :else (print-usage))))


(defn dispatch
  "find which function to run"
  [fn args]
  (cond
    (= fn "1") (d1 args)
    (= fn "2") (d2 args)
    (= fn "3") (d3 args)
    :else (print-usage)))

(defn -main
  "cli entry"
  [& args]
  (dispatch (first args) (rest args)))
