(define (domain robot_navigation)
  (:requirements :strips :typing)
  
  (:types 
    robot zone
  )
  
  (:predicates 
    (robot_at ?r - robot ?z - zone)
    (can-move ?from - zone ?to - zone)
  )
  
  (:action move
    :parameters (?r - robot ?from - zone ?to - zone)
    :precondition (and 
      (robot_at ?r ?from) 
      (can-move ?from ?to)
    )
    :effect (and 
      (not (robot_at ?r ?from)) 
      (robot_at ?r ?to)
    )
  )
)