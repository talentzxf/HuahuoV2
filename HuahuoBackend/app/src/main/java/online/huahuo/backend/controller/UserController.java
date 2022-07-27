package online.huahuo.backend.controller;

import online.huahuo.backend.db.UserEntity;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.exception.UserNotFoundException;
import org.springframework.web.bind.annotation.*;

@RestController
public class UserController {
    private final UserRepository userRepository;


    public UserController(UserRepository userRepository) {
        this.userRepository = userRepository;
    }

    @GetMapping("/user/{id}")
    UserEntity one(@PathVariable Long id){
        return userRepository.findById(id).orElseThrow( ()-> new UserNotFoundException(id));
    }

    @PostMapping("/user")
    UserEntity newUser(@RequestBody UserEntity userEntity){
        return userRepository.save(userEntity);
    }
}
