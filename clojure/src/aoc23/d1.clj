(ns aoc23.d1
  (:require [clojure.string :as s]))

(def num-start (int \0))
(def num-end (int \9))
(defn is-digit
  [c]
  (and (>= c num-start) (<= c num-end)))

(defn digit-to-num [c] (- c num-start))
(defn num-from-pair [pair]
  (+ (* 10 (first pair)) (second pair)))


(defn take-lastone [col]
  (if (or (nil? col) (empty? col)) nil
      (let [col-next (next col)
            take-last-aux (fn [curr, n]
                            (if (or (nil? n) (empty? n)) curr
                                (recur (next curr) (next n))))]
        (take-last-aux col col-next))))

(defn first-and-last
  [col]
  (cons
   (first col)
   (take-lastone col)))

;; (defn print_line_detail [line]
;;   (if (nil? line) nil
;;       (do (println (first line))
;;           (print_line_detail (rest line)))))

(defn num-from-line
  [line]
  (->> line
       (filter (comp not nil?))
       (map int)
       (filter is-digit)
       first-and-last
       (map digit-to-num)
       num-from-pair))

;; (defn seq-len [seq]
;;   (reduce (fn [acc _] (+ acc 1)) 0 seq))

;; (defn seq-rm [seq rem]
  ;; (drop (seq-len rem) seq))

(defn seq-cmp [seq cmp]
  (cond (nil? cmp) true
    (nil? seq) false
    (not= (first seq) (first cmp)) false
    :else (recur (next seq) (next cmp))))
(defn seq-cmp-map [seq cmp-map]
  (if (nil? seq) nil
      (let [cmp-res
            (->> cmp-map
                 (reduce (fn [res, [entry, cmp]]
                           (assoc res entry (seq-cmp seq cmp)))
                         {})
                 (filter (fn [[_,res]] (identity res))))]
        (if (empty? cmp-res) nil
            (first (first cmp-res))))))

(declare seq-cmp-map-store-first)

(defn seq-cmp-map-store [[seq word-map digit-map fst rev]]
  (if (nil? seq) {:first fst :last (first rev)}
      (let [curr-digit (->>
                        digit-map
                        (reduce (fn [k1, [k2,v]]
                                  (if (= v (first seq)) k2 k1)) nil)
                        (get digit-map))]
        (if (nil? curr-digit)
          (let [rep-key (seq-cmp-map seq word-map)]
            (if (nil? rep-key)
              (seq-cmp-map-store-first
               (list (next seq) word-map digit-map fst rev))
              (seq-cmp-map-store-first
               (list (next seq) word-map digit-map fst
                     (cons (get digit-map rep-key) rev)))))
          (seq-cmp-map-store-first
           (list (next seq) word-map digit-map fst
                       (cons curr-digit rev)))))))
(defn seq-cmp-map-store-first
  [[seq word-map digit-map fst rev]]
  (if (and (nil? fst) (some? rev))
    (seq-cmp-map-store (list seq word-map digit-map (first rev) rev))
    (seq-cmp-map-store (list seq word-map digit-map fst rev))))

(defn seq-map-nums-res [seq word-map digit-map]
  (let [tramp-res
        (trampoline seq-cmp-map-store
                    (list seq word-map digit-map nil nil))
        c1 (:first tramp-res) c2 (:last tramp-res)]
    (if (or (nil? c1) (nil? c2))
      0
      (+ (* 10 (-> c1 int digit-to-num))
         (-> c2 int digit-to-num)))))
;; (defn seq-cmp-map-replace [seq cmp-map rep-map]
;;   (if (nil? seq) nil
;;       (let [rep-key (seq-cmp-map seq cmp-map)]
;;         (if (nil? rep-key)
;;           (cons (first seq) (seq-cmp-map-replace
;;                              (next seq) cmp-map rep-map))
;;           (cons (get rep-map rep-key)
;;                 (seq-cmp-map-replace
;;                  (seq-rm seq (get cmp-map rep-key))
;;                  cmp-map rep-map))))))

;; (defn replace-words-with-digits [line]
;;   (seq-cmp-map-replace
;;    line
;;    {:a "one" :b "two" :c "three" :d "four" :e "five"
;;     :f "six" :g "seven" :h "eight" :i "nine"}
;;    {:a \1 :b \2 :c \3 :d \4 :e \5 :f \6 :g \7 :h \8 :i \9}))

;; (defn prep-output [lines]
;;   (s/join \newline lines))

(defn part-1 [lines]
  (->> lines
       (map num-from-line)
       (reduce +)))

(defn part-2 [lines]
  (->> lines
       (map
        #(seq-map-nums-res %
{:a "one" :b "two" :c "three" :d "four" :e "five"
 :f "six" :g "seven" :h "eight" :i "nine"}
   {:a \1 :b \2 :c \3 :d \4 :e \5 :f \6 :g \7 :h \8 :i \9}))
       (reduce +)))
