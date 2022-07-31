package online.huahuo.backend.db;

import org.springframework.data.repository.CrudRepository;

public interface UserRepository extends CrudRepository<UserDB, Long> {
    UserDB findByIdentifier(String identifier);
}
