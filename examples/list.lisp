(printl (list 1 2 3))
(printl (cons 1 (cons 2 (cons 3 nil))))

(printn (car (list 1 2 3)))
(printl (cdr (list 1 2 3)))

(defun twice (xs) (ilist) ilist
  (cond ilist
   ((nil? xs) nil)
   ((cons (* (car xs) 2) (twice (cdr xs))))))

(printl (twice (list 1 2 3)))
