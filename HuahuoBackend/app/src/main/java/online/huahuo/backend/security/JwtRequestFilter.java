package online.huahuo.backend.security;

import io.jsonwebtoken.JwtException;
import io.jsonwebtoken.MalformedJwtException;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.db.UserRole;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.authority.SimpleGrantedAuthority;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.web.authentication.WebAuthenticationDetailsSource;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import static java.util.function.Predicate.not;

@Component
@RequiredArgsConstructor
public class JwtRequestFilter extends OncePerRequestFilter{
    private static final String AUTHORIZATION_HEADER = "Authorization";
    private static final Pattern BEARER_PATTERN = Pattern.compile("^Bearer (.+?)$");

    private static final String SECURITY_DEFAULT_PREFIX = "ROLE_";
    @Autowired
    private JwtTokenUtil jwtTokenUtil;

    @Autowired
    private UserRepository userRepository;

    private Optional<String> getToken(HttpServletRequest request){
        return Optional
                .ofNullable(request.getHeader(AUTHORIZATION_HEADER))
                .filter(not(String::isEmpty))
                .map(BEARER_PATTERN::matcher)
                .filter(Matcher::find)
                .map(matcher -> matcher.group(1));
    }

    JwtUserDetails getUserDetails(UserDB user, String token){
        List<SimpleGrantedAuthority> authorities = new ArrayList<>(1);
        authorities.add(new SimpleGrantedAuthority(user.getRole().toString()));
        if(user.getRole().equals(UserRole.ADMIN)){
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX  + UserRole.ADMIN.name()));
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX  + UserRole.CREATOR.name()));
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX  + UserRole.READER.name()));
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX  + UserRole.ANONYMOUS.name()));
        }
        else if(user.getRole().equals(UserRole.CREATOR)){
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX + UserRole.CREATOR.name()));
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX + UserRole.READER.name()));
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX + UserRole.ANONYMOUS.name()));
        } else if(user.getRole().equals(UserRole.READER)){
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX + UserRole.READER.name()));
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX + UserRole.ANONYMOUS.name()));
        }else{
            authorities.add(new SimpleGrantedAuthority(SECURITY_DEFAULT_PREFIX + UserRole.ANONYMOUS.name()));
        }

        return JwtUserDetails.builder()
                .username(user.getUsername())
                .password(user.getPassword())
                .authorities(authorities)
                .token(token)
                .build();
    }

    JwtUserDetails loadUserByToken(String token){
        String username = jwtTokenUtil.getUsernameFromToken(token);
        UserDB userDB = userRepository.findByUsername(username);
        return getUserDetails(userDB, token);
    }

    @Override
    protected void doFilterInternal(HttpServletRequest request, HttpServletResponse response, FilterChain filterChain) throws ServletException, IOException {
        try {
            getToken(request)
                    .map(this::loadUserByToken)
                    .map(jwtUserDetails -> JwtPreAuthenticationToken
                            .builder()
                            .principal(jwtUserDetails)
                            .details(new WebAuthenticationDetailsSource().buildDetails(request))
                            .build())
                    .ifPresent(authentication -> SecurityContextHolder.getContext().setAuthentication(authentication));

            filterChain.doFilter(request, response);
        }catch(JwtException ex){
            response.sendError(HttpServletResponse.SC_UNAUTHORIZED);
        }
    }
}