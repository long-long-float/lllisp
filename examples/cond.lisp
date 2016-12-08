(print
 (cond string
  ((= 1 1) "OK")
  ("NG")))

(print
 (cond string
  ((= 1 2) "NG")
  ("OK")))

(cond string
 ((= 1 1) (progn
   (print "OK1")
   (print "OK2")
  ))
 ((print "NG")))

(print
 (cond string
  ((= 1 2) "NG")
  ((= 3 4) "NG")
  ("OK")
  ))
