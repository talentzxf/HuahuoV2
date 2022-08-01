package online.huahuo.backend.security;

import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.userdetails.User;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.stereotype.Service;

import java.util.ArrayList;

@Service
public class JwtUserDetailsService implements UserDetailsService {
    @Autowired
    UserRepository userRepository;

    @Override
    public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
        UserDB userDB = userRepository.findByUsername(username);
        if(userDB == null){
            throw new UsernameNotFoundException("Can't find user:" + username);
        }

        return new User(username, userDB.getPassword(), new ArrayList<>());
    }
}
