(ns aoc23.string)

(defn getnext [s]
  (if (= (first s) \space) (recur (next s)) s))

(defn trim [s c]
  (if (= (first s) c) (recur (next s) c) s))

(defn is-digit [c]
  (and (>= (int c) (int \0)) (<= (int c) (int \9))))
(defn conv-digit [c] (- (int c) (int \0)))

(defn getnum [s]
  (let [s (getnext s)
        anc (fn [s carry]
              (let [c (first s)]
                (if (and (some? c) (is-digit c))
                  (recur (next s)
                         (+ (* 10 carry) (conv-digit c)))
                  carry)))]
        (anc s 0)))

(defn matchnum
  "takes from str until next char is not a digit"
  [s] (if (and (-> s first some?) (-> s first is-digit))
        (recur (next s)) s))

(defn checknum
  "returns true if s contains a number after possible whitespace"
  [s]
  (if (-> s getnext nil?) false
    (-> s getnext first is-digit)))

(defn checkstr [s, pred]
  (let [s (getnext s)
        anc (fn [s p]
              (cond
                (nil? p) true
                (nil? s) false
                (not= (first s) (first p)) false
                :else (recur (next s) (next p))))]
    (anc s pred)))
(defn strlen [s]
  (let [anc (fn [r s] (if (nil? s) r (recur (+ 1 r) (next s))))]
    (anc 0 s)))
(defn stepstr [s n] (if (= 0 n) s (recur (next s) (- n 1))))
(defn matchstr [s match] (stepstr s (strlen match)))
