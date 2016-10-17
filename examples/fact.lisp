(defun fact (n)
 (cond
  ((> n 2) (* n (fact (- n 1))))
  (n)
  ))

(printn (fact 5))
