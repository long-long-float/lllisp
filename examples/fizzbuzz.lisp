(defun fizzbuzz (n)
 (cond ((> 101 n) (progn
   (print
    (cond
     ((= (mod n 15) 0) "FizzBuzz")
     ((= (mod n 3)  0) "Fizz")
     ((= (mod n 5)  0) "Buzz")
     ((itoa n))
     ))
   (fizzbuzz (+ n 1))))
  ((print "finish"))))

(fizzbuzz 1)
