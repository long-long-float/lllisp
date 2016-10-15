(defun fizzbuzz (n)
 (cond ((> 100 n) (progn
  (print
   (cond
    ((eq (mod n 15) 0) "FizzBuzz")
    ((eq (mod n 3)  0) "Fizz")
    ((eq (mod n 5)  0) "Buzz")
    (n)
    ))
  (fizzbuzz (+ n 1))))
  (t)))

(fizzbuzz 1)
