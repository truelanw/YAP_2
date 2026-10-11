void
lock_acquire (struct lock *lock)
{
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (!lock_held_by_current_thread (lock));

  struct thread *cur = thread_current ();

  if (lock->holder != NULL)
    {
      cur->waiting_lock = lock;
      donate_priority (cur, lock->holder);
    }

  sema_down (&lock->semaphore);
  
  lock->holder = cur;
  cur->waiting_lock = NULL;

  list_push_back (&cur->locks, &lock->lock_elem);
}

void
lock_release (struct lock *lock) 
{
  ASSERT (lock != NULL);
  ASSERT (lock_held_by_current_thread (lock));

  list_remove (&lock->lock_elem);
  
  lock->holder = NULL;

  refresh_priority (thread_current ());
  
  sema_up (&lock->semaphore);
}