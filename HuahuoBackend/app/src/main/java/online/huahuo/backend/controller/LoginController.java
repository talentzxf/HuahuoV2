package online.huahuo.backend.controller;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;
import io.jsonwebtoken.SignatureAlgorithm;
import lombok.Data;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.db.UserStatus;
import online.huahuo.backend.exception.UserNotFoundException;
import online.huahuo.backend.security.JwtTokenUtil;
import online.huahuo.backend.security.SecurityProperties;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import javax.crypto.SecretKey;
import java.time.Instant;
import java.util.Date;
import java.util.Optional;

@Data
class LoginStatus{
    private String userName;
    private String failReason;
    private String jwtToken;
    private HttpStatus httpStatus;
}

@RestController
@RequiredArgsConstructor
public class LoginController {
    private final UserRepository userRepository;
    private final AuthenticationProvider authenticationProvider;
    private final SecurityProperties properties;
    private final JwtTokenUtil jwtTokenUtil;

    @Transactional
    public String issueToken(Long userId) {
        Optional<UserDB> userOptional = userRepository.findById(userId);
        if (userOptional.isEmpty()) throw new UserNotFoundException(userId);

        UserDB user = userOptional.get();
        return jwtTokenUtil.generateToken(user);
    }

    @PostMapping("/login")
    ResponseEntity<?> login(@RequestParam(required = false) String username, @RequestParam(required = false) String password){
        LoginStatus loginStatus = new LoginStatus();
        if(username == null || password == null){
            loginStatus.setFailReason("Username or pwd is null");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        loginStatus.setUserName(username);

        UserDB user = userRepository.findByUsername(username);
        if(user == null){
            loginStatus.setFailReason("Can't find user!");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        if(user.getStatus() != UserStatus.ACTIVE){
            loginStatus.setFailReason("User is not active!");
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        String token = issueToken(user.getId());

        loginStatus.setJwtToken(token);
        UsernamePasswordAuthenticationToken credentials = new UsernamePasswordAuthenticationToken(username, password);
        SecurityContextHolder.getContext().setAuthentication(authenticationProvider.authenticate(credentials));

        loginStatus.setHttpStatus(HttpStatus.OK);
        return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
    }
}
