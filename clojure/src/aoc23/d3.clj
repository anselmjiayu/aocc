(ns aoc23.d3
  (:require [aoc23.string :as s]))

(declare get-line-sum)

;; (defn part-1
;;   [lines]
;;   (->> lines
;;        (map get-line-sum)
;;        (reduce + 0)))

(def empty-pos \.)
(def empty-str (repeat empty-pos))
(def nil-str (repeat nil))

(defn add-delimit-lines [lines]
  (concat [empty-str] lines [empty-str]))

(defn third
  "aka caddr"
  [col]
  (second (next col)))

(defn stagger-lines [lines]
  (if (nil? lines) nil
      (cons (vector (first lines) (second lines) (third lines))
            (stagger-lines (next lines)))))

(defn take-first-5 [linesq]
  (->> linesq
       (map #(take 5 %))
       (map (fn [s] (->> s (map #(take 5 %)))))))

(defn is-symbol [c]
  (and (some? c) (not (s/is-digit c)) (not= c empty-pos)))

(defn parse-symbol
  "takes a symbol from the stream, returning a token and stream after the token"
  [s pos]
  (cond (nil? s) [nil nil]
    (is-symbol (first s))
    [[(first s) pos (inc pos)] (next s)]
    :else (recur (next s) (inc pos))
      )
  )

(defn parse-number
  "takes a number from the stream, returning a token and stream after the token"
  [s pos]
  (if (nil? s) [nil nil]
      (let [c (first s)
            nxt (next s)
            anc (fn [s pos start carry]
                  (let [c (first s) nxt (next s)]
                    (if (or (nil? c) (not (s/is-digit c)))
                      [[carry start pos] s]
                      (recur nxt (inc pos) start (+ (s/conv-digit c)
                                                    (* 10 carry)))
                      )))]
        (if (s/is-digit c)
          (anc s pos pos 0)
          (recur nxt (inc pos))
          ))))


(defn token-seq
  "produce token seq with parse function"
  [s, pf]
  (let [anc (fn anc [s pos]
              (let [[t, nxt] (pf s pos)]
                (if (nil? t) nil
                  (cons t (anc nxt (third t))))))]
    (anc s 0)
    ))

;; (defn tokens-adj?
;;   "returns true if a number token and a symbol token are adjacent"
;;   [num-t sym-t]
;;    (and (>= (second num-t) (third sym-t))
;;         (<= (third num-t) (second sym-t))))

(defn tokens-adj?
  [num-t sym-t]
   (cond (< (third num-t) (second sym-t)) :adv-num
     (> (second num-t) (third sym-t)) :adv-sym
     :else :adj
     ))

(defn part-seq [num-ts sym-ts]
  (let [num-t (first num-ts) sym-t (first sym-ts)
        num-nxt (next sym-ts) sym-nxt (next sym-ts)]
    (if (or (nil? num-t) (nil? sym-t)) nil
      (case (tokens-adj? num-t sym-t)
        :adj (cons (first num-t)
                   (part-seq num-nxt sym-ts))
        :adv-num (recur num-nxt sym-ts)
        :adv-sym (recur num-ts sym-nxt)
      )
    )))

(defn sym-ts-from [s]
  (if nil? (first s) nil-str)
  (token-seq s parse-symbol))

(defn num-ts-from [s]
  (if nil? (first s) nil-str)
  (token-seq s parse-symbol))

(defn line-parts-sum
  [[prev-line, curr-line, nxt-line]]
  (let [symts-prv (sym-ts-from prev-line)
        symts-cur (sym-ts-from curr-line)
        symts-nxt (sym-ts-from nxt-line)
        numts (num-ts-from curr-line)
        prv-seq (part-seq numts symts-prv)
        cur-seq (part-seq numts symts-cur)
        nxt-seq (part-seq numts symts-nxt)
        sum #(reduce + 0 %)
        ]
  (sum [(sum prv-seq) (sum cur-seq) (sum nxt-seq)]))
  )

(defn part-1 [lines]
  (let [lines (concat [nil-str] [lines] [nil-str])]
    (->> lines (map line-parts-sum) (reduce + 0))
))



;; (defn read-num-from-lines
;;   "reads a number and records validity so far"
;;   [[carry, valid], [prv, cur, nxt]]
;;   (let [c (first cur)]
;;     (if (s/is-digit c)
;;       (if (and ()not valid
;;                (or (is-symbol (first prv))
;;                    (is-symbol (first nxt))))
;;         (recur
;;          [(+ (s/conv-digit c) (* 10 carry)) true]
;;           [(next prv) (next cur) (next nxt)])
;;         (recur
;;          [(+ (s/conv-digit c) (* 10 carry)) valid]
;;           [(next prv) (next cur) (next nxt)])
;;       )))


;; (defn get-line-sum
;;   [line]
;;   (comp ))
