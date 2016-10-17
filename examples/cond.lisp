(print
 (cond
  ((= 1 1) "OK")
  ("NG")))

(print
 (cond
  ((= 1 2) "NG")
  ("OK")))

(cond
 ((= 1 1) (progn
   (print "OK1")
   (print "OK2")
  ))
 ("NG"))
